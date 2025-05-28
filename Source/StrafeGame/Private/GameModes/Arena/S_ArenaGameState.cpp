#include "GameModes/Arena/S_ArenaGameState.h"
#include "Net/UnrealNetwork.h"
#include "GameModes/Arena/S_ArenaPlayerState.h" // For CurrentLeaderPlayerState example

AS_ArenaGameState::AS_ArenaGameState()
{
    FragLimit = 0; // Default, will be set by GameMode
    MatchStateNameOverride = NAME_None;
    MatchDurationSeconds = 0;
    // CurrentLeaderPlayerState = nullptr;
}

void AS_ArenaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); // Replicates RemainingTime from base
    DOREPLIFETIME(AS_ArenaGameState, FragLimit);
    DOREPLIFETIME(AS_ArenaGameState, MatchStateNameOverride);
    DOREPLIFETIME(AS_ArenaGameState, MatchDurationSeconds);
    // DOREPLIFETIME(AS_ArenaGameState, CurrentLeaderPlayerState);
}

void AS_ArenaGameState::SetMatchStateNameOverride(FName NewName)
{
    if (HasAuthority())
    {
        MatchStateNameOverride = NewName;
        OnRep_MatchStateNameOverride(); // Call on server too for local listeners
    }
}

void AS_ArenaGameState::OnRep_MatchStateNameOverride()
{
    OnMatchStateNameOverrideChangedDelegate.Broadcast(MatchStateNameOverride);
}


// Example for leader tracking
/*
void AS_ArenaGameState::OnRep_CurrentLeader()
{
    // Broadcast delegate or update UI elements that show the current leader
}

void AS_ArenaGameState::UpdateLeader()
{
    if (HasAuthority())
    {
        AS_ArenaPlayerState* NewLeader = nullptr;
        int32 MaxFrags = -1;

        for (APlayerState* PS : PlayerArray)
        {
            AS_ArenaPlayerState* ArenaPS = Cast<AS_ArenaPlayerState>(PS);
            if (ArenaPS)
            {
                if (ArenaPS->GetFrags() > MaxFrags)
                {
                    MaxFrags = ArenaPS->GetFrags();
                    NewLeader = ArenaPS;
                }
                else if (ArenaPS->GetFrags() == MaxFrags && MaxFrags != -1)
                {
                    NewLeader = nullptr; // Draw, no single leader
                }
            }
        }

        if (CurrentLeaderPlayerState != NewLeader)
        {
            CurrentLeaderPlayerState = NewLeader;
            OnRep_CurrentLeader(); // For server-side immediate effect and to trigger replication
        }
    }
}
*/