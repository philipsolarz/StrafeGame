#include "GameModes/Strafe/S_StrafeManager.h"
#include "GameModes/Strafe/Actors/S_CheckpointTrigger.h" // For AS_CheckpointTrigger
#include "Player/S_Character.h"
#include "GameModes/Strafe/S_StrafePlayerState.h" // For AS_StrafePlayerState
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h" // For GetAllActorsOfClass
#include "Engine/World.h"           // For GetWorld()

AS_StrafeManager::AS_StrafeManager()
{
    PrimaryActorTick.bCanEverTick = false; // Can be true if it needs to manage timed events not tied to players
    bReplicates = true;
    SetReplicateMovement(false); // This actor likely won't move

    StartLine = nullptr;
    FinishLine = nullptr;
    TotalCheckpointsForFullLap = 0;
}

void AS_StrafeManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AS_StrafeManager, AllCheckpointsInOrder);
    DOREPLIFETIME(AS_StrafeManager, Scoreboard);
    // StartLine and FinishLine are derived from AllCheckpointsInOrder, no need to replicate separately if clients can also derive.
    // Or, replicate them if clients need them explicitly without iterating. For now, not replicating them directly.
}

void AS_StrafeManager::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        // Initial scan for checkpoints. GameMode might call this again if level setup can change.
        RefreshAndInitializeCheckpoints();
    }
}

void AS_StrafeManager::RefreshAndInitializeCheckpoints()
{
    if (!HasAuthority()) return;

    // Clear previous checkpoint data
    for (AS_CheckpointTrigger* CP : AllCheckpointsInOrder)
    {
        if (CP)
        {
            CP->OnCheckpointReachedDelegate.RemoveDynamic(this, &AS_StrafeManager::HandleCheckpointReached);
        }
    }
    AllCheckpointsInOrder.Empty();
    StartLine = nullptr;
    FinishLine = nullptr;
    TotalCheckpointsForFullLap = 0;

    TArray<AActor*> FoundCheckpointActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AS_CheckpointTrigger::StaticClass(), FoundCheckpointActors);

    UE_LOG(LogTemp, Log, TEXT("AS_StrafeManager: Found %d checkpoint actors in the level."), FoundCheckpointActors.Num());

    for (AActor* Actor : FoundCheckpointActors)
    {
        AS_CheckpointTrigger* CP = Cast<AS_CheckpointTrigger>(Actor);
        if (CP)
        {
            RegisterCheckpoint(CP);
        }
    }
    FinalizeCheckpointSetup(); // Sorts and identifies start/finish
}

void AS_StrafeManager::RegisterCheckpoint(AS_CheckpointTrigger* Checkpoint)
{
    // This function is called by RefreshAndInitializeCheckpoints, which already checks HasAuthority()
    if (Checkpoint && !AllCheckpointsInOrder.Contains(Checkpoint))
    {
        AllCheckpointsInOrder.Add(Checkpoint);
        Checkpoint->OnCheckpointReachedDelegate.AddUniqueDynamic(this, &AS_StrafeManager::HandleCheckpointReached);
        UE_LOG(LogTemp, Log, TEXT("AS_StrafeManager: Registered Checkpoint '%s' (Order: %d, Type: %s)"),
            *Checkpoint->GetName(),
            Checkpoint->GetCheckpointOrder(),
            *UEnum::GetValueAsString(Checkpoint->GetCheckpointType())
        );
    }
}

void AS_StrafeManager::SortCheckpoints()
{
    AllCheckpointsInOrder.Sort([](const AS_CheckpointTrigger& A, const AS_CheckpointTrigger& B) {
        return A.GetCheckpointOrder() < B.GetCheckpointOrder();
        });
}

void AS_StrafeManager::FinalizeCheckpointSetup()
{
    if (!HasAuthority()) return;

    SortCheckpoints();

    if (AllCheckpointsInOrder.Num() > 0)
    {
        bool bFoundStart = false;
        bool bFoundFinish = false;

        for (AS_CheckpointTrigger* CP : AllCheckpointsInOrder)
        {
            if (CP)
            {
                if (CP->GetCheckpointType() == ECheckpointType::Start && !bFoundStart)
                {
                    StartLine = CP;
                    bFoundStart = true;
                }
                else if (CP->GetCheckpointType() == ECheckpointType::Finish && !bFoundFinish)
                {
                    FinishLine = CP;
                    bFoundFinish = true;
                }
            }
        }

        if (!bFoundStart && AllCheckpointsInOrder.Num() > 0)
        {
            // Attempt to infer StartLine if only one checkpoint has order 0 and is not Finish
            for (AS_CheckpointTrigger* CP : AllCheckpointsInOrder) {
                if (CP && CP->GetCheckpointOrder() == 0 && CP->GetCheckpointType() != ECheckpointType::Finish) {
                    StartLine = CP;
                    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: No explicit Start Line found. Using checkpoint '%s' (Order 0) as Start."), *StartLine->GetName());
                    break;
                }
            }
            if (!StartLine) UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager: No checkpoint marked as 'Start' or with Order 0. Please mark one."));
        }

        if (!bFoundFinish && AllCheckpointsInOrder.Num() > 0)
        {
            // Attempt to infer FinishLine if only one checkpoint has highest order and is not Start
            AS_CheckpointTrigger* PotentialFinish = AllCheckpointsInOrder.Last();
            if (PotentialFinish && PotentialFinish != StartLine && PotentialFinish->GetCheckpointType() != ECheckpointType::Start) {
                FinishLine = PotentialFinish;
                UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: No explicit Finish Line found. Using checkpoint '%s' (Highest Order) as Finish."), *FinishLine->GetName());
            }
            if (!FinishLine && AllCheckpointsInOrder.Num() > 1) UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager: No checkpoint clearly identifiable as 'Finish'. Please mark one."));
        }

        if (StartLine) UE_LOG(LogTemp, Log, TEXT("AS_StrafeManager: Start Line identified: %s"), *StartLine->GetName());
        if (FinishLine) UE_LOG(LogTemp, Log, TEXT("AS_StrafeManager: Finish Line identified: %s"), *FinishLine->GetName());


        if (StartLine && FinishLine && StartLine == FinishLine && AllCheckpointsInOrder.Num() > 1)
        {
            UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager: Start and Finish line cannot be the same checkpoint actor if there are multiple checkpoints."));
            FinishLine = nullptr; // Invalidate if ambiguous
        }
        TotalCheckpointsForFullLap = AllCheckpointsInOrder.Num();
        UE_LOG(LogTemp, Log, TEXT("AS_StrafeManager: Checkpoint setup finalized. Total Checkpoints (incl. Start/Finish if separate): %d"), TotalCheckpointsForFullLap);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: No checkpoints found or registered during FinalizeCheckpointSetup."));
    }
}


void AS_StrafeManager::HandleCheckpointReached(AS_CheckpointTrigger* Checkpoint, AS_Character* PlayerCharacter)
{
    if (!HasAuthority() || !PlayerCharacter || !Checkpoint || !StartLine || !FinishLine)
    {
        // UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager::HandleCheckpointReached - Early exit due to invalid params or setup."));
        return;
    }

    AS_StrafePlayerState* StrafePS = PlayerCharacter->GetPlayerState<AS_StrafePlayerState>();
    if (!StrafePS)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager::HandleCheckpointReached - PlayerCharacter %s has no AS_StrafePlayerState."), *PlayerCharacter->GetName());
        return;
    }

    const int32 CheckpointIdxInSortedList = AllCheckpointsInOrder.IndexOfByKey(Checkpoint);
    if (CheckpointIdxInSortedList == INDEX_NONE)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager::HandleCheckpointReached - Reached checkpoint %s not in AllCheckpointsInOrder list!"), *Checkpoint->GetName());
        return;
    }

    // Porting logic from old ARaceManager::HandleCheckpointReached
    if (Checkpoint->GetCheckpointType() == ECheckpointType::Start)
    {
        // If race is not active, this is a fresh start
        if (!StrafePS->IsRaceInProgress())
        {
            StrafePS->ServerResetRaceState(); // Ensure clean state
            StrafePS->ServerStartRace();
            // ServerReachedCheckpoint will use the index from AllCheckpointsInOrder
            StrafePS->ServerReachedCheckpoint(CheckpointIdxInSortedList, TotalCheckpointsForFullLap);
        }
        // If race is active AND they are hitting the start line sequentially (e.g. completing a lap)
        // This logic depends on how laps are counted. If start is also the finish for lap completion,
        // then hitting start after finish might mean a new lap.
        // For now, assume hitting start when race is active might just reset if out of sequence.
        // The original ARaceManager had complex logic for this; simplifying for now.
        // A common pattern: if you hit start and LastCheckpointReached was the finish line's index, it's a new lap.
        else if (StrafePS->IsRaceInProgress() && StrafePS->GetLastCheckpointReached() == AllCheckpointsInOrder.IndexOfByKey(FinishLine))
        {
            UE_LOG(LogTemp, Log, TEXT("AS_StrafeManager: Player %s completed a lap and hit Start Line again. Starting new lap."), *StrafePS->GetPlayerNameOrSpectator());
            StrafePS->ServerResetRaceState(); // Reset for new lap (keeps best time)
            StrafePS->ServerStartRace();
            StrafePS->ServerReachedCheckpoint(CheckpointIdxInSortedList, TotalCheckpointsForFullLap); // Log start line for new lap
        }
        // Else, if they hit Start mid-race out of order, it might be ignored or reset them.
        // For now, if race is active and it's not a new lap, we might just re-trigger ServerStartRace
        // to effectively reset their current attempt if they go backwards.
        else if (StrafePS->IsRaceInProgress())
        {
            UE_LOG(LogTemp, Log, TEXT("AS_StrafeManager: Player %s hit Start Line mid-race out of sequence. Resetting current run."), *StrafePS->GetPlayerNameOrSpectator());
            StrafePS->ServerResetRaceState();
            StrafePS->ServerStartRace();
            StrafePS->ServerReachedCheckpoint(CheckpointIdxInSortedList, TotalCheckpointsForFullLap);
        }
    }
    else if (Checkpoint->GetCheckpointType() == ECheckpointType::Finish)
    {
        if (StrafePS->IsRaceInProgress())
        {
            // Player must have hit all previous checkpoints in order.
            // The index of the finish line should be TotalCheckpointsForFullLap - 1
            if (CheckpointIdxInSortedList == TotalCheckpointsForFullLap - 1 &&
                StrafePS->GetLastCheckpointReached() == CheckpointIdxInSortedList - 1) // Must have hit previous one
            {
                StrafePS->ServerReachedCheckpoint(CheckpointIdxInSortedList, TotalCheckpointsForFullLap); // Log the finish line itself
                StrafePS->ServerFinishedRace(CheckpointIdxInSortedList, TotalCheckpointsForFullLap);
                UpdatePlayerInScoreboard(StrafePS);
            }
            else {
                UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: Player %s hit Finish Line out of sequence. LastCP: %d, FinishCP Index: %d, Expected Prev: %d"),
                    *StrafePS->GetPlayerNameOrSpectator(), StrafePS->GetLastCheckpointReached(), CheckpointIdxInSortedList, CheckpointIdxInSortedList - 1);
            }
        }
    }
    else // ECheckpointType::Checkpoint (Intermediate)
    {
        if (StrafePS->IsRaceInProgress())
        {
            // Player must hit intermediate checkpoints in order
            if (StrafePS->GetLastCheckpointReached() == CheckpointIdxInSortedList - 1)
            {
                StrafePS->ServerReachedCheckpoint(CheckpointIdxInSortedList, TotalCheckpointsForFullLap);
            }
            else {
                UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: Player %s hit Intermediate Checkpoint %s out of sequence. LastCP: %d, CP Index: %d, Expected Prev: %d"),
                    *StrafePS->GetPlayerNameOrSpectator(), *Checkpoint->GetName(), StrafePS->GetLastCheckpointReached(), CheckpointIdxInSortedList, CheckpointIdxInSortedList - 1);
            }
        }
    }
}

void AS_StrafeManager::UpdatePlayerInScoreboard(AS_PlayerState* PlayerStateBase)
{
    if (!HasAuthority() || !PlayerStateBase) return;

    AS_StrafePlayerState* StrafePS = Cast<AS_StrafePlayerState>(PlayerStateBase);
    if (!StrafePS) return;

    const FPlayerStrafeRaceTime& CurrentBestTime = StrafePS->GetBestRaceTime();
    if (!CurrentBestTime.IsValid()) return; // Don't add to scoreboard if no valid best time yet

    int32 EntryIndex = Scoreboard.IndexOfByPredicate([StrafePS](const FPlayerScoreboardEntry_Strafe& Entry) {
        return Entry.PlayerStateRef == StrafePS;
        });

    if (EntryIndex != INDEX_NONE) // Existing entry
    {
        // Update if new best time is better than current scoreboard entry, or if scoreboard entry was invalid
        if (!Scoreboard[EntryIndex].BestTime.IsValid() || CurrentBestTime.TotalTime < Scoreboard[EntryIndex].BestTime.TotalTime)
        {
            Scoreboard[EntryIndex].BestTime = CurrentBestTime;
            UE_LOG(LogTemp, Log, TEXT("AS_StrafeManager Scoreboard: Updated best time for %s to %f"), *StrafePS->GetPlayerNameOrSpectator(), CurrentBestTime.TotalTime);
        }
    }
    else // New entry
    {
        FPlayerScoreboardEntry_Strafe NewEntry;
        NewEntry.PlayerName = StrafePS->GetPlayerNameOrSpectator().ToString();
        NewEntry.BestTime = CurrentBestTime;
        NewEntry.PlayerStateRef = StrafePS;
        Scoreboard.Add(NewEntry);
        UE_LOG(LogTemp, Log, TEXT("AS_StrafeManager Scoreboard: Added %s with best time %f"), *NewEntry.PlayerName, NewEntry.BestTime.TotalTime);
    }

    Scoreboard.Sort(); // Sort by best total time (ascending, using FPlayerScoreboardEntry_Strafe::operator<)
    OnRep_Scoreboard(); // Manually call for server & trigger client replication
}

void AS_StrafeManager::OnRep_Scoreboard()
{
    OnScoreboardUpdatedDelegate.Broadcast();
    // UE_LOG(LogTemp, Log, TEXT("AS_StrafeManager: Scoreboard replicated/updated. Entries: %d"), Scoreboard.Num());
    // if (Scoreboard.Num() > 0)
    // {
    //      UE_LOG(LogTemp, Log, TEXT("AS_StrafeManager: Top Score: %s - %.3f"), *Scoreboard[0].PlayerName, Scoreboard[0].BestTime.TotalTime);
    // }
}