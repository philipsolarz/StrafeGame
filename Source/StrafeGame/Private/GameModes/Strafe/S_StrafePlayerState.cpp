#include "GameModes/Strafe/S_StrafePlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

AS_StrafePlayerState::AS_StrafePlayerState()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    CurrentRaceTime = 0.0f;
    LastCheckpointReached = -1;
    bIsRaceActiveForPlayer = false;
    BestRaceTime.Reset();
}

void AS_StrafePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(AS_StrafePlayerState, CurrentRaceTime, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(AS_StrafePlayerState, CurrentSplitTimes, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(AS_StrafePlayerState, CurrentSplitDeltas, COND_None, REPNOTIFY_Always);
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
    }
}

void AS_StrafePlayerState::Reset()
{
    Super::Reset();
    if (HasAuthority())
    {
        ServerResetRaceState();
    }
}

void AS_StrafePlayerState::CopyProperties(APlayerState* InPlayerState)
{
    Super::CopyProperties(InPlayerState);
    AS_StrafePlayerState* SourceStrafePS = Cast<AS_StrafePlayerState>(InPlayerState);
    if (SourceStrafePS && HasAuthority())
    {
        this->BestRaceTime = SourceStrafePS->BestRaceTime;
    }
}

void AS_StrafePlayerState::ServerStartRace()
{
    if (HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("[STRAFE DEBUG] PlayerState '%s': ServerStartRace called."), *GetPlayerName());
        ServerResetRaceState();
        bIsRaceActiveForPlayer = true;
        SetActorTickEnabled(true); // <-- CRITICAL FIX: Enable tick to count time.
        OnRep_IsRaceActiveForPlayer();
        BroadcastStrafeStateUpdate();
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

            float Delta = 0.0f;
            if (BestRaceTime.IsValid() && BestRaceTime.SplitTimes.IsValidIndex(LastCheckpointReached))
            {
                Delta = CurrentRaceTime - BestRaceTime.SplitTimes[LastCheckpointReached];
            }
            CurrentSplitDeltas.Add(Delta);

            UE_LOG(LogTemp, Warning, TEXT("[STRAFE DEBUG] PlayerState '%s': Reached Checkpoint %d at time %f."), *GetPlayerName(), CheckpointIndex, CurrentRaceTime);

            OnRep_LastCheckpointReached();
            OnRep_CurrentSplitTimes();
            OnRep_CurrentSplitDeltas();
            OnStrafePlayerCheckpointHitDelegate.Broadcast(CheckpointIndex, CurrentRaceTime);
            BroadcastStrafeStateUpdate();
        }
    }
}

void AS_StrafePlayerState::ServerFinishedRace(int32 FinalCheckpointIndex, int32 TotalCheckpointsInRace)
{
    if (HasAuthority() && bIsRaceActiveForPlayer)
    {
        if (LastCheckpointReached == FinalCheckpointIndex)
        {
            UE_LOG(LogTemp, Warning, TEXT("[STRAFE DEBUG] PlayerState '%s': ServerFinishedRace at time %f."), *GetPlayerName(), CurrentRaceTime);
            bIsRaceActiveForPlayer = false;
            SetActorTickEnabled(false);

            if (!BestRaceTime.IsValid() || CurrentRaceTime < BestRaceTime.TotalTime)
            {
                UE_LOG(LogTemp, Warning, TEXT("[STRAFE DEBUG] PlayerState '%s': New best time!"), *GetPlayerName());
                BestRaceTime.TotalTime = CurrentRaceTime;
                BestRaceTime.SplitTimes = CurrentSplitTimes;
                OnRep_BestRaceTime();
            }

            OnRep_IsRaceActiveForPlayer();
            OnStrafePlayerFinishedRaceDelegate.Broadcast(CurrentRaceTime);
            BroadcastStrafeStateUpdate();
        }
    }
}

void AS_StrafePlayerState::ServerResetRaceState()
{
    if (HasAuthority())
    {
        CurrentRaceTime = 0.0f;
        CurrentSplitTimes.Empty();
        CurrentSplitDeltas.Empty();
        LastCheckpointReached = -1;
        bIsRaceActiveForPlayer = false;
        SetActorTickEnabled(false);

        OnRep_CurrentRaceTime();
        OnRep_CurrentSplitTimes();
        OnRep_CurrentSplitDeltas();
        OnRep_LastCheckpointReached();
        OnRep_IsRaceActiveForPlayer();
        BroadcastStrafeStateUpdate();
    }
}

void AS_StrafePlayerState::OnRep_CurrentRaceTime() { BroadcastStrafeStateUpdate(); }
void AS_StrafePlayerState::OnRep_CurrentSplitTimes() { BroadcastStrafeStateUpdate(); }
void AS_StrafePlayerState::OnRep_CurrentSplitDeltas() { BroadcastStrafeStateUpdate(); }
void AS_StrafePlayerState::OnRep_BestRaceTime()
{
    OnStrafePlayerNewBestTimeDelegate.Broadcast(BestRaceTime);
    BroadcastStrafeStateUpdate();
}
void AS_StrafePlayerState::OnRep_LastCheckpointReached()
{
    BroadcastStrafeStateUpdate();
}
void AS_StrafePlayerState::OnRep_IsRaceActiveForPlayer()
{
    if (bIsRaceActiveForPlayer)
    {
        OnStrafePlayerRaceStartedDelegate.Broadcast();
    }
    BroadcastStrafeStateUpdate();
}
void AS_StrafePlayerState::BroadcastStrafeStateUpdate()
{
    OnStrafePlayerRaceStateChangedDelegate.Broadcast(this);
}