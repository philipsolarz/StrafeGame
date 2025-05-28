#include "GameModes/Strafe/S_StrafePlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h" // For GetWorld()
// No TimerManager.h needed if using Tick for the race timer

AS_StrafePlayerState::AS_StrafePlayerState()
{
    // Enable ticking for this PlayerState if a race is active (server-side)
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false; // Only enable when race starts

    CurrentRaceTime = 0.0f;
    LastCheckpointReached = -1;
    bIsRaceActiveForPlayer = false;
    BestRaceTime.Reset();
}

void AS_StrafePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    //DOREPLIFETIME_CONDITION(AS_StrafePlayerState, CurrentRaceTime, COND_None, REPNOTIFY_Always); // COND_None as clients need this for UI
    //DOREPLIFETIME_CONDITION(AS_StrafePlayerState, CurrentSplitTimes, COND_None, REPNOTIFY_Always);
    //DOREPLIFETIME_CONDITION(AS_StrafePlayerState, BestRaceTime, COND_None, REPNOTIFY_Always);
    //DOREPLIFETIME_CONDITION(AS_StrafePlayerState, LastCheckpointReached, COND_None, REPNOTIFY_Always);
    //DOREPLIFETIME_CONDITION(AS_StrafePlayerState, bIsRaceActiveForPlayer, COND_None, REPNOTIFY_Always);

    DOREPLIFETIME_CONDITION_NOTIFY(AS_StrafePlayerState, CurrentRaceTime, COND_None, REPNOTIFY_Always); // COND_None as clients need this for UI
    DOREPLIFETIME_CONDITION_NOTIFY(AS_StrafePlayerState, CurrentSplitTimes, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(AS_StrafePlayerState, BestRaceTime, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(AS_StrafePlayerState, LastCheckpointReached, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(AS_StrafePlayerState, bIsRaceActiveForPlayer, COND_None, REPNOTIFY_Always);
}

void AS_StrafePlayerState::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (HasAuthority() && bIsRaceActiveForPlayer)
    {
        CurrentRaceTime += DeltaSeconds;
        // CurrentRaceTime will be replicated. UI can poll or use OnRep_CurrentRaceTime for less frequent updates.
        // If extremely smooth client-side timer is needed, client can run its own cosmetic timer started by OnRep_bIsRaceActiveForPlayer.
    }
}

void AS_StrafePlayerState::Reset()
{
    Super::Reset(); // Handles base class resets (e.g., bIsDead)

    if (HasAuthority())
    {
        CurrentRaceTime = 0.0f;
        CurrentSplitTimes.Empty();
        // BestRaceTime is persistent across resets unless explicitly cleared by game mode logic
        LastCheckpointReached = -1;
        bIsRaceActiveForPlayer = false;
        SetActorTickEnabled(false);

        // Manually call OnReps on server to reflect state change immediately for local listeners and mark for replication
        OnRep_CurrentRaceTime();
        OnRep_CurrentSplitTimes();
        OnRep_LastCheckpointReached();
        OnRep_IsRaceActiveForPlayer(); // This will also broadcast relevant delegates
    }
    // BestRaceTime is intentionally not reset here, as it's typically persistent.
    // GameMode can call a specific function to clear best times if needed.
}

void AS_StrafePlayerState::CopyProperties(APlayerState* InPlayerState)
{
    Super::CopyProperties(InPlayerState);

    AS_StrafePlayerState* SourceStrafePS = Cast<AS_StrafePlayerState>(InPlayerState);
    if (SourceStrafePS)
    {
        if (HasAuthority()) // Server copies the authoritative state
        {
            this->CurrentRaceTime = SourceStrafePS->CurrentRaceTime;
            this->CurrentSplitTimes = SourceStrafePS->CurrentSplitTimes;
            this->BestRaceTime = SourceStrafePS->BestRaceTime;
            this->LastCheckpointReached = SourceStrafePS->LastCheckpointReached;
            this->bIsRaceActiveForPlayer = SourceStrafePS->bIsRaceActiveForPlayer;
            SetActorTickEnabled(this->bIsRaceActiveForPlayer);
            // OnRep functions will be called naturally due to replication on clients
        }
    }
}

void AS_StrafePlayerState::ServerStartRace()
{
    if (HasAuthority())
    {
        UE_LOG(LogTemp, Log, TEXT("AS_StrafePlayerState for %s: ServerStartRace called."), *GetPlayerName());
        CurrentRaceTime = 0.0f;
        CurrentSplitTimes.Empty();
        LastCheckpointReached = -1;
        bIsRaceActiveForPlayer = true;
        SetActorTickEnabled(true); // Start ticking on server

        // Call OnRep for server itself & to mark for replication.
        OnRep_IsRaceActiveForPlayer(); // Broadcasts OnStrafePlayerRaceStartedDelegate
        OnRep_CurrentRaceTime();
        OnRep_CurrentSplitTimes();
        OnRep_LastCheckpointReached();

        BroadcastStrafeStateUpdate(); // Generic state change
    }
}

void AS_StrafePlayerState::ServerReachedCheckpoint(int32 CheckpointIndex, int32 TotalCheckpointsInRace)
{
    if (HasAuthority() && bIsRaceActiveForPlayer)
    {
        if (CheckpointIndex == LastCheckpointReached + 1)
        {
            LastCheckpointReached = CheckpointIndex;
            CurrentSplitTimes.Add(CurrentRaceTime);
            UE_LOG(LogTemp, Log, TEXT("AS_StrafePlayerState for %s: Reached checkpoint %d at time %f. Total Splits: %d"),
                *GetPlayerName(), CheckpointIndex, CurrentRaceTime, CurrentSplitTimes.Num());

            OnRep_LastCheckpointReached(); // Will call OnStrafePlayerCheckpointHitDelegate on reps
            OnRep_CurrentSplitTimes();     // Updates client splits

            OnStrafePlayerCheckpointHitDelegate.Broadcast(CheckpointIndex, CurrentRaceTime); // Server-side direct broadcast
            BroadcastStrafeStateUpdate();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_StrafePlayerState for %s: Attempted to hit checkpoint %d out of order. Last hit: %d"),
                *GetPlayerName(), CheckpointIndex, LastCheckpointReached);
        }
    }
}

void AS_StrafePlayerState::ServerFinishedRace(int32 FinalCheckpointIndex, int32 TotalCheckpointsInRace)
{
    if (HasAuthority() && bIsRaceActiveForPlayer)
    {
        // Similar logic to your old RaceStateComponent
        if (LastCheckpointReached == FinalCheckpointIndex && CurrentSplitTimes.Num() == TotalCheckpointsInRace)
        {
            bIsRaceActiveForPlayer = false;
            SetActorTickEnabled(false); // Stop ticking on server
            UE_LOG(LogTemp, Log, TEXT("AS_StrafePlayerState for %s: Finished race at time %f."),
                *GetPlayerName(), CurrentRaceTime);

            if (!BestRaceTime.IsValid() || CurrentRaceTime < BestRaceTime.TotalTime)
            {
                UE_LOG(LogTemp, Log, TEXT("AS_StrafePlayerState for %s: NEW BEST TIME! Old: %f, New: %f"),
                    *GetPlayerName(), BestRaceTime.TotalTime, CurrentRaceTime);
                BestRaceTime.TotalTime = CurrentRaceTime;
                BestRaceTime.SplitTimes = CurrentSplitTimes;
                OnRep_BestRaceTime(); // Broadcasts OnStrafePlayerNewBestTimeDelegate
            }

            OnRep_IsRaceActiveForPlayer(); // Broadcasts OnStrafePlayerFinishedRaceDelegate (due to bIsRaceActiveForPlayer being false)
            OnStrafePlayerFinishedRaceDelegate.Broadcast(CurrentRaceTime); // Explicit server-side broadcast
            BroadcastStrafeStateUpdate();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AS_StrafePlayerState for %s: ServerFinishedRace - CONDITIONS NOT MET. LastCP: %d (Expected %d), Splits.Num(): %d (Expected %d)"),
                *GetPlayerName(), LastCheckpointReached, FinalCheckpointIndex, CurrentSplitTimes.Num(), TotalCheckpointsInRace);
        }
    }
}

void AS_StrafePlayerState::ServerResetRaceState()
{
    if (HasAuthority())
    {
        UE_LOG(LogTemp, Log, TEXT("AS_StrafePlayerState for %s: ServerResetRaceState called."), *GetPlayerName());
        CurrentRaceTime = 0.0f;
        CurrentSplitTimes.Empty();
        LastCheckpointReached = -1;
        bIsRaceActiveForPlayer = false;
        SetActorTickEnabled(false); // Stop ticking on server

        OnRep_CurrentRaceTime();
        OnRep_CurrentSplitTimes();
        OnRep_LastCheckpointReached();
        OnRep_IsRaceActiveForPlayer();
        BroadcastStrafeStateUpdate();
    }
}

void AS_StrafePlayerState::OnRep_CurrentRaceTime()
{
    BroadcastStrafeStateUpdate();
}

void AS_StrafePlayerState::OnRep_CurrentSplitTimes()
{
    BroadcastStrafeStateUpdate();
    if (LastCheckpointReached >= 0 && CurrentSplitTimes.IsValidIndex(LastCheckpointReached))
    {
        // Ensure client UI also gets checkpoint hit event when splits data arrives
        OnStrafePlayerCheckpointHitDelegate.Broadcast(LastCheckpointReached, CurrentSplitTimes[LastCheckpointReached]);
    }
}

void AS_StrafePlayerState::OnRep_BestRaceTime()
{
    OnStrafePlayerNewBestTimeDelegate.Broadcast(BestRaceTime);
    BroadcastStrafeStateUpdate();
}

void AS_StrafePlayerState::OnRep_LastCheckpointReached()
{
    BroadcastStrafeStateUpdate();
    if (LastCheckpointReached >= 0 && CurrentSplitTimes.IsValidIndex(LastCheckpointReached))
    {
        // Ensure client UI also gets checkpoint hit event if LastCheckpointReached replicates
        OnStrafePlayerCheckpointHitDelegate.Broadcast(LastCheckpointReached, CurrentSplitTimes[LastCheckpointReached]);
    }
}

void AS_StrafePlayerState::OnRep_IsRaceActiveForPlayer()
{
    if (HasAuthority()) // Server manages its own tick state directly
    {
        SetActorTickEnabled(bIsRaceActiveForPlayer);
    }
    // For clients, this OnRep is informational for UI.

    if (bIsRaceActiveForPlayer)
    {
        OnStrafePlayerRaceStartedDelegate.Broadcast();
    }
    // If race becomes inactive, the OnStrafePlayerFinishedRaceDelegate is broadcast by ServerFinishedRace logic.
    // Resetting also sets bIsRaceActiveForPlayer to false, effectively signaling race stop.
    BroadcastStrafeStateUpdate();
}

void AS_StrafePlayerState::BroadcastStrafeStateUpdate()
{
    OnStrafePlayerRaceStateChangedDelegate.Broadcast(this);
}