// Source/StrafeGame/Private/GameModes/Strafe/S_StrafeManager.cpp
#include "GameModes/Strafe/S_StrafeManager.h"
#include "GameModes/Strafe/Actors/S_CheckpointTrigger.h" 
#include "Player/S_Character.h"
#include "GameModes/Strafe/S_StrafePlayerState.h" 
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h" 
#include "Engine/World.h"           

AS_StrafeManager::AS_StrafeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(false);

    StartLine = nullptr;
    FinishLine = nullptr;
    TotalCheckpointsForFullLap = 0;
}

void AS_StrafeManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AS_StrafeManager, AllCheckpointsInOrder);
    DOREPLIFETIME(AS_StrafeManager, Scoreboard);
}

void AS_StrafeManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager::BeginPlay - Manager created and ready"));
}

void AS_StrafeManager::RefreshAndInitializeCheckpoints()
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager::RefreshAndInitializeCheckpoints called on client. Ignoring."));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager::RefreshAndInitializeCheckpoints - Starting checkpoint initialization"));

    // Clear existing bindings
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

    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: Found %d checkpoint actors in the level."), FoundCheckpointActors.Num());

    for (AActor* Actor : FoundCheckpointActors)
    {
        AS_CheckpointTrigger* CP = Cast<AS_CheckpointTrigger>(Actor);
        if (CP)
        {
            RegisterCheckpoint(CP);
        }
    }
    FinalizeCheckpointSetup();
}

void AS_StrafeManager::RegisterCheckpoint(AS_CheckpointTrigger* Checkpoint)
{
    if (Checkpoint && !AllCheckpointsInOrder.Contains(Checkpoint))
    {
        AllCheckpointsInOrder.Add(Checkpoint);

        // Bind to the checkpoint's delegate
        Checkpoint->OnCheckpointReachedDelegate.AddUniqueDynamic(this, &AS_StrafeManager::HandleCheckpointReached);

        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: Registered Checkpoint '%s' (Order: %d, Type: %s) and bound delegate."),
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
            AS_CheckpointTrigger* PotentialFinish = AllCheckpointsInOrder.Last();
            if (PotentialFinish && PotentialFinish != StartLine && PotentialFinish->GetCheckpointType() != ECheckpointType::Start) {
                FinishLine = PotentialFinish;
                UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: No explicit Finish Line found. Using checkpoint '%s' (Highest Order) as Finish."), *FinishLine->GetName());
            }
            if (!FinishLine && AllCheckpointsInOrder.Num() > 1) UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager: No checkpoint clearly identifiable as 'Finish'. Please mark one."));
        }

        if (StartLine)
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: Start Line identified: %s"), *StartLine->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager: CRITICAL - No Start Line found! Race cannot function!"));
        }

        if (FinishLine)
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: Finish Line identified: %s"), *FinishLine->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager: CRITICAL - No Finish Line found! Race cannot function!"));
        }

        if (StartLine && FinishLine && StartLine == FinishLine && AllCheckpointsInOrder.Num() > 1)
        {
            UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager: Start and Finish line cannot be the same checkpoint actor if there are multiple checkpoints."));
            FinishLine = nullptr;
        }

        TotalCheckpointsForFullLap = AllCheckpointsInOrder.Num();
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: Checkpoint setup finalized. Total Checkpoints: %d"), TotalCheckpointsForFullLap);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager: No checkpoints found or registered during FinalizeCheckpointSetup."));
    }
}

void AS_StrafeManager::HandleCheckpointReached(AS_CheckpointTrigger* Checkpoint, AS_Character* PlayerCharacter)
{
    UE_LOG(LogTemp, Warning, TEXT("[STRAFE DEBUG] StrafeManager: HandleCheckpointReached called. Checkpoint: %s, Player: %s"),
        Checkpoint ? *Checkpoint->GetName() : TEXT("NULL"),
        PlayerCharacter ? *PlayerCharacter->GetName() : TEXT("NULL"));

    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager::HandleCheckpointReached called on client. Ignoring."));
        return;
    }

    if (!PlayerCharacter || !Checkpoint)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager::HandleCheckpointReached - Invalid parameters. Player: %s, Checkpoint: %s"),
            PlayerCharacter ? TEXT("Valid") : TEXT("NULL"),
            Checkpoint ? TEXT("Valid") : TEXT("NULL"));
        return;
    }

    if (!StartLine || !FinishLine)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager::HandleCheckpointReached - Race setup incomplete! StartLine: %s, FinishLine: %s"),
            StartLine ? TEXT("Valid") : TEXT("NULL"),
            FinishLine ? TEXT("Valid") : TEXT("NULL"));
        return;
    }

    // Add detailed PlayerState debugging
    APlayerState* GenericPS = PlayerCharacter->GetPlayerState();
    if (!GenericPS)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager::HandleCheckpointReached - PlayerCharacter %s has NO PlayerState at all!"), *PlayerCharacter->GetName());

        // Check if character has a controller
        AController* Controller = PlayerCharacter->GetController();
        if (Controller)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - Character has Controller: %s"), *Controller->GetName());
            APlayerState* ControllerPS = Controller->GetPlayerState<APlayerState>();
            if (ControllerPS)
            {
                UE_LOG(LogTemp, Warning, TEXT("  - Controller has PlayerState: %s of class %s"), *ControllerPS->GetName(), *ControllerPS->GetClass()->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("  - Controller has NO PlayerState"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  - Character has NO Controller"));
        }
        return;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager::HandleCheckpointReached - PlayerCharacter %s has PlayerState: %s of class %s"),
            *PlayerCharacter->GetName(), *GenericPS->GetName(), *GenericPS->GetClass()->GetName());
    }

    AS_StrafePlayerState* StrafePS = Cast<AS_StrafePlayerState>(GenericPS);
    if (!StrafePS)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager::HandleCheckpointReached - PlayerState exists but is not AS_StrafePlayerState! Actual class: %s"), *GenericPS->GetClass()->GetName());
        return;
    }

    // Rest of the function remains the same...
    const int32 CheckpointIdxInSortedList = AllCheckpointsInOrder.IndexOfByKey(Checkpoint);
    if (CheckpointIdxInSortedList == INDEX_NONE)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StrafeManager::HandleCheckpointReached - Reached checkpoint %s not in AllCheckpointsInOrder list!"), *Checkpoint->GetName());
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[STRAFE DEBUG] Processing checkpoint %d for player %s. Race active: %s"),
        CheckpointIdxInSortedList, *StrafePS->GetPlayerName(), StrafePS->IsRaceInProgress() ? TEXT("YES") : TEXT("NO"));

    if (Checkpoint->GetCheckpointType() == ECheckpointType::Start)
    {
        if (!StrafePS->IsRaceInProgress())
        {
            UE_LOG(LogTemp, Warning, TEXT("[STRAFE DEBUG] Starting race for player %s"), *StrafePS->GetPlayerName());
            StrafePS->ServerStartRace();
            StrafePS->ServerReachedCheckpoint(CheckpointIdxInSortedList, TotalCheckpointsForFullLap);
        }
        else if (StrafePS->IsRaceInProgress() && StrafePS->GetLastCheckpointReached() == AllCheckpointsInOrder.IndexOfByKey(FinishLine))
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: Player %s completed a lap and hit Start Line again. Starting new lap."), *StrafePS->GetPlayerName());
            StrafePS->ServerStartRace();
            StrafePS->ServerReachedCheckpoint(CheckpointIdxInSortedList, TotalCheckpointsForFullLap);
        }
        else if (StrafePS->IsRaceInProgress())
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: Player %s hit Start Line mid-race out of sequence. Resetting current run."), *StrafePS->GetPlayerName());
            StrafePS->ServerStartRace();
            StrafePS->ServerReachedCheckpoint(CheckpointIdxInSortedList, TotalCheckpointsForFullLap);
        }
    }
    else if (Checkpoint->GetCheckpointType() == ECheckpointType::Finish)
    {
        if (StrafePS->IsRaceInProgress())
        {
            if (CheckpointIdxInSortedList == TotalCheckpointsForFullLap - 1 &&
                StrafePS->GetLastCheckpointReached() == CheckpointIdxInSortedList - 1)
            {
                UE_LOG(LogTemp, Warning, TEXT("[STRAFE DEBUG] Player %s finished race!"), *StrafePS->GetPlayerName());
                StrafePS->ServerReachedCheckpoint(CheckpointIdxInSortedList, TotalCheckpointsForFullLap);
                StrafePS->ServerFinishedRace(CheckpointIdxInSortedList, TotalCheckpointsForFullLap);
                UpdatePlayerInScoreboard(StrafePS);
            }
            else {
                UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: Player %s hit Finish Line out of sequence. LastCP: %d, FinishCP Index: %d, Expected Prev: %d"),
                    *StrafePS->GetPlayerName(), StrafePS->GetLastCheckpointReached(), CheckpointIdxInSortedList, CheckpointIdxInSortedList - 1);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: Player %s hit Finish Line but race not in progress"), *StrafePS->GetPlayerName());
        }
    }
    else // Intermediate checkpoint
    {
        if (StrafePS->IsRaceInProgress())
        {
            if (StrafePS->GetLastCheckpointReached() == CheckpointIdxInSortedList - 1)
            {
                UE_LOG(LogTemp, Warning, TEXT("[STRAFE DEBUG] Player %s reached checkpoint %d"), *StrafePS->GetPlayerName(), CheckpointIdxInSortedList);
                StrafePS->ServerReachedCheckpoint(CheckpointIdxInSortedList, TotalCheckpointsForFullLap);
            }
            else {
                UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: Player %s hit Intermediate Checkpoint %s out of sequence. LastCP: %d, CP Index: %d, Expected Prev: %d"),
                    *StrafePS->GetPlayerName(), *Checkpoint->GetName(), StrafePS->GetLastCheckpointReached(), CheckpointIdxInSortedList, CheckpointIdxInSortedList - 1);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager: Player %s hit checkpoint but race not in progress"), *StrafePS->GetPlayerName());
        }
    }
}

void AS_StrafeManager::UpdatePlayerInScoreboard(AS_PlayerState* PlayerStateBase)
{
    if (!HasAuthority() || !PlayerStateBase) return;

    AS_StrafePlayerState* StrafePS = Cast<AS_StrafePlayerState>(PlayerStateBase);
    if (!StrafePS) return;

    const FPlayerStrafeRaceTime& CurrentBestTime = StrafePS->GetBestRaceTime();
    if (!CurrentBestTime.IsValid()) return;

    int32 EntryIndex = Scoreboard.IndexOfByPredicate([StrafePS](const FPlayerScoreboardEntry_Strafe& Entry) {
        return Entry.PlayerStateRef == StrafePS;
        });

    if (EntryIndex != INDEX_NONE)
    {
        if (!Scoreboard[EntryIndex].BestTime.IsValid() || CurrentBestTime.TotalTime < Scoreboard[EntryIndex].BestTime.TotalTime)
        {
            Scoreboard[EntryIndex].BestTime = CurrentBestTime;
            UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager Scoreboard: Updated best time for %s to %f"), *StrafePS->GetPlayerName(), CurrentBestTime.TotalTime);
        }
    }
    else
    {
        FPlayerScoreboardEntry_Strafe NewEntry;
        NewEntry.PlayerName = StrafePS->GetPlayerName();
        NewEntry.BestTime = CurrentBestTime;
        NewEntry.PlayerStateRef = StrafePS;
        Scoreboard.Add(NewEntry);
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeManager Scoreboard: Added %s with best time %f"), *NewEntry.PlayerName, NewEntry.BestTime.TotalTime);
    }

    Scoreboard.Sort();
    OnRep_Scoreboard();
}

void AS_StrafeManager::OnRep_Scoreboard()
{
    OnScoreboardUpdatedDelegate.Broadcast();
}