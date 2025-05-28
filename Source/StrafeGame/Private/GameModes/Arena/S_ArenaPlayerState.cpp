#include "GameModes/Arena/S_ArenaPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "GameModes/Arena/S_ArenaGameMode.h" // Example: If you need to notify the ArenaGameMode directly
#include "Engine/World.h"                   // For GetWorld()

AS_ArenaPlayerState::AS_ArenaPlayerState()
{
    Frags = 0;
    Deaths = 0;
}

void AS_ArenaPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AS_ArenaPlayerState, Frags);
    DOREPLIFETIME(AS_ArenaPlayerState, Deaths);
}

void AS_ArenaPlayerState::Reset()
{
    Super::Reset(); // Calls base class Reset (handles bIsDead, core GAS re-init if any)

    // Reset Arena-specific scores
    if (HasAuthority()) // Score changes should be server-authoritative
    {
        Frags = 0;
        Deaths = 0;
        BroadcastArenaScoreUpdate(); // Ensure OnRep_ functions are effectively called on server for local listeners and to mark for replication.
    }
    // For clients, OnRep functions will handle updates when replicated values change.
}

void AS_ArenaPlayerState::CopyProperties(APlayerState* InPlayerState)
{
    Super::CopyProperties(InPlayerState);

    AS_ArenaPlayerState* SourceArenaPS = Cast<AS_ArenaPlayerState>(InPlayerState);
    if (SourceArenaPS)
    {
        // Only copy if on server, replication will handle clients
        if (HasAuthority())
        {
            this->Frags = SourceArenaPS->Frags;
            this->Deaths = SourceArenaPS->Deaths;
            // Broadcast update if values actually changed, or let replication handle it.
            // Since this is a direct copy, usually just setting the values is enough, replication handles the rest.
        }
    }
}

void AS_ArenaPlayerState::ScoreFrag(AS_PlayerState* KillerPlayerState, AS_PlayerState* VictimPlayerState)
{
    if (HasAuthority() && this == KillerPlayerState && KillerPlayerState != VictimPlayerState)
    {
        Frags++;
        BroadcastArenaScoreUpdate();

        // Optional: Notify ArenaGameMode directly if it needs immediate reaction beyond polling
        // AS_ArenaGameMode* ArenaGM = GetWorld() ? GetWorld()->GetAuthGameMode<AS_ArenaGameMode>() : nullptr;
        // if (ArenaGM)
        // {
        //     ArenaGM->OnPlayerScoredFrag(this);
        // }
    }
}

void AS_ArenaPlayerState::ScoreDeath(AS_PlayerState* KilledPlayerState, AS_PlayerState* KillerPlayerState)
{
    if (HasAuthority() && this == KilledPlayerState)
    {
        Deaths++;
        BroadcastArenaScoreUpdate();
    }
}

void AS_ArenaPlayerState::ServerResetScore()
{
    if (HasAuthority())
    {
        Frags = 0;
        Deaths = 0;
        BroadcastArenaScoreUpdate();
    }
}

void AS_ArenaPlayerState::OnRep_Frags()
{
    BroadcastArenaScoreUpdate();
}

void AS_ArenaPlayerState::OnRep_Deaths()
{
    BroadcastArenaScoreUpdate();
}

void AS_ArenaPlayerState::BroadcastArenaScoreUpdate()
{
    // Broadcast the specific delegate for this class
    OnArenaScoreUpdatedDelegate.Broadcast(this, Frags, Deaths);
    // UE_LOG(LogTemp, Log, TEXT("AS_ArenaPlayerState %s: Score Update Broadcast - Frags: %d, Deaths: %d"), *GetPlayerNameOrSpectator(), Frags, Deaths);
}