#include "GameModes/Arena/S_ArenaGameState.h"
#include "Net/UnrealNetwork.h"
#include "GameModes/Arena/S_ArenaPlayerState.h" 
#include "UI/ViewModels/S_KillfeedItemViewModel.h" // Ensures FKillfeedEventData is known

AS_ArenaGameState::AS_ArenaGameState()
{
    FragLimit = 0;
    MatchStateNameOverride = NAME_None;
    MatchDurationSeconds = 0;
}

void AS_ArenaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AS_ArenaGameState, FragLimit);
    DOREPLIFETIME(AS_ArenaGameState, MatchStateNameOverride);
    DOREPLIFETIME(AS_ArenaGameState, MatchDurationSeconds);
    DOREPLIFETIME(AS_ArenaGameState, RecentKillsLog);
}

void AS_ArenaGameState::SetMatchStateNameOverride(FName NewName)
{
    if (HasAuthority())
    {
        MatchStateNameOverride = NewName;
        OnRep_MatchStateNameOverride();
    }
}

void AS_ArenaGameState::OnRep_MatchStateNameOverride()
{
    OnMatchStateNameOverrideChangedDelegate.Broadcast(MatchStateNameOverride);
}

void AS_ArenaGameState::OnRep_RecentKills()
{
    OnKillfeedUpdated.Broadcast();
}

// Corrected function signature to match the header
void AS_ArenaGameState::AddKillToLog(const FKillfeedEventData& KillInfo)
{
    if (HasAuthority()) // This is AActor::HasAuthority(), called on 'this' instance
    {
        // KillInfo is now correctly typed as FKillfeedEventData
        RecentKillsLog.Insert(KillInfo, 0);

        const int32 MaxLogSize = 5;
        if (RecentKillsLog.Num() > MaxLogSize)
        {
            RecentKillsLog.RemoveAt(MaxLogSize, RecentKillsLog.Num() - MaxLogSize);
        }

        OnRep_RecentKills(); // This is this->OnRep_RecentKills(), correct
    }
}