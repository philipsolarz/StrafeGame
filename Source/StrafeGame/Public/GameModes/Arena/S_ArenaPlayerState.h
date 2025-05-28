#pragma once

#include "CoreMinimal.h"
#include "Player/S_PlayerState.h" // Inherit from our base PlayerState
#include "Delegates/DelegateCombinations.h" // For DECLARE_DYNAMIC_MULTICAST_DELEGATE
#include "S_ArenaPlayerState.generated.h"

// Forward declaration
class AS_ArenaGameMode; // If ArenaPlayerState needs to interact with ArenaGameMode

// Delegate for when the Arena score (Frags or Deaths) changes specifically for this ArenaPlayerState.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnArenaScoreUpdatedDelegate, AS_ArenaPlayerState*, PlayerState, int32, NewFrags, int32, NewDeaths);

UCLASS(Blueprintable, Config = Game)
class STRAFEGAME_API AS_ArenaPlayerState : public AS_PlayerState
{
    GENERATED_BODY()

public:
    AS_ArenaPlayerState();

    //~ Begin AActor Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    //~ End AActor Interface

    //~ Begin APlayerState Interface
    /** Called when the PlayerState is reset (e.g., player changes team, game restarts). */
    virtual void Reset() override;
    /** Override to copy properties from the old PlayerState to the new one during seamless travel. */
    virtual void CopyProperties(APlayerState* PlayerState) override;
    //~ End APlayerState Interface

    /**
     * Called by the GameMode (server-side) when this player scores a frag.
     * @param KillerPlayerState The PlayerState of the killer (usually this player).
     * @param VictimPlayerState The PlayerState of the victim.
     */
    virtual void ScoreFrag(AS_PlayerState* KillerPlayerState, AS_PlayerState* VictimPlayerState);

    /**
     * Called by the GameMode (server-side) when this player dies.
     * @param KilledPlayerState The PlayerState of the player who died (usually this player).
     * @param KillerPlayerState The PlayerState of the killer (can be null or self).
     */
    virtual void ScoreDeath(AS_PlayerState* KilledPlayerState, AS_PlayerState* KillerPlayerState);

    /** Gets the current number of frags. */
    UFUNCTION(BlueprintPure, Category = "ArenaPlayerState|Score")
    int32 GetFrags() const { return Frags; }

    /** Gets the current number of deaths. */
    UFUNCTION(BlueprintPure, Category = "ArenaPlayerState|Score")
    int32 GetDeaths() const { return Deaths; }

    /** Resets frags and deaths to zero. Server-only. */
    UFUNCTION(BlueprintCallable, Category = "ArenaPlayerState|Score", meta = (DisplayName = "Reset Arena Score (Server)"))
    void ServerResetScore();

    /** Delegate broadcast when frags or deaths change for this player. */
    UPROPERTY(BlueprintAssignable, Category = "ArenaPlayerState|Events")
    FOnArenaScoreUpdatedDelegate OnArenaScoreUpdatedDelegate;

protected:
    UPROPERTY(Transient, ReplicatedUsing = OnRep_Frags)
    int32 Frags;

    UPROPERTY(Transient, ReplicatedUsing = OnRep_Deaths)
    int32 Deaths;

    UFUNCTION()
    virtual void OnRep_Frags();

    UFUNCTION()
    virtual void OnRep_Deaths();

    /** Called internally when score changes to broadcast the specific Arena delegate. */
    virtual void BroadcastArenaScoreUpdate();
};