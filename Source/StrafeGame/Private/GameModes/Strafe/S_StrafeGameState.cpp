#include "GameModes/Strafe/S_StrafeGameState.h"
#include "GameModes/Strafe/S_StrafeManager.h" // For AS_StrafeManager
#include "Net/UnrealNetwork.h"

AS_StrafeGameState::AS_StrafeGameState()
{
    StrafeManager = nullptr;
    MatchDurationSeconds = 0; // Will be set by GameMode
    CurrentMatchStateName = NAME_None;
}

void AS_StrafeGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); // Replicates RemainingTime from base
    DOREPLIFETIME(AS_StrafeGameState, StrafeManager);
    DOREPLIFETIME(AS_StrafeGameState, MatchDurationSeconds);
    DOREPLIFETIME(AS_StrafeGameState, CurrentMatchStateName);
}

void AS_StrafeGameState::SetStrafeManager(AS_StrafeManager* InStrafeManager)
{
    if (HasAuthority())
    {
        StrafeManager = InStrafeManager;
        // Could call OnRep manually if server needs to react to it immediately
        // OnRep_StrafeManager();
    }
}

void AS_StrafeGameState::OnRep_StrafeManager()
{
    // Clients now know about the StrafeManager.
    // UI or other client systems can access it if needed.
    // UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameState: Client OnRep_StrafeManager - %s"), StrafeManager ? *StrafeManager->GetName() : TEXT("NULL"));
}

void AS_StrafeGameState::SetCurrentMatchStateName(FName NewStateName)
{
    if (HasAuthority())
    {
        if (CurrentMatchStateName != NewStateName)
        {
            CurrentMatchStateName = NewStateName;
            OnRep_CurrentMatchStateName(); // Call for server + replication
        }
    }
}

void AS_StrafeGameState::OnRep_CurrentMatchStateName()
{
    OnCurrentMatchStateNameChangedDelegate.Broadcast(CurrentMatchStateName);
    // UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameState: Replicated Match State Name changed to %s"), *CurrentMatchStateName.ToString());
}