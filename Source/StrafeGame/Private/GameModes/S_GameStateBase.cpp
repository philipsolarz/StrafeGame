#include "GameModes/S_GameStateBase.h"
#include "Net/UnrealNetwork.h" // For DOREPLIFETIME

AS_GameStateBase::AS_GameStateBase()
{
    RemainingTime = 0;
    // MaxPlayers = 16; // Example default
}

void AS_GameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AS_GameStateBase, RemainingTime);
    // DOREPLIFETIME(AS_GameStateBase, MaxPlayers);
}

void AS_GameStateBase::OnRep_RemainingTime()
{
    OnRemainingTimeChangedDelegate.Broadcast(RemainingTime);
}

void AS_GameStateBase::SetRemainingTime(int32 NewTime)
{
    if (HasAuthority())
    {
        RemainingTime = FMath::Max(0, NewTime);
        // If called on server, OnRep will not fire automatically for the server itself.
        // If server needs to react to this change immediately:
        OnRemainingTimeChangedDelegate.Broadcast(RemainingTime);
    }
}