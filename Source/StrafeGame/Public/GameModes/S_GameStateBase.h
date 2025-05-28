#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h" // Using AGameState for its PlayerArray and replication features
#include "S_GameStateBase.generated.h"

/**
 * Base GameState for StrafeGame.
 * Tracks common game state information replicated to all clients.
 */
UCLASS(Abstract, MinimalAPI)
class AS_GameStateBase : public AGameState
{
    GENERATED_BODY()

public:
    AS_GameStateBase();

    //~ Begin AGameStateBase Interface
    /** Returns the time an FName based match state has been active. */
    // virtual float GetMatchStateTimeRemaining() const override; // This is from AGameMode, not AGameState
    //~ End AGameStateBase Interface

    /**
     * Current human-readable name of the match state (e.g., "Warmup", "InProgress", "PostMatch").
     * This is replicated from AGameModeBase::GetMatchState() via AGameStateBase::MatchState.
     * This getter is just for convenience.
     */
    UFUNCTION(BlueprintPure, Category = "GameState")
    FName GetMatchStateName() const { return GetMatchState(); } // GetMatchState() is from AGameStateBase

    /** Time remaining in the current state or match, in seconds. Replicated. */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RemainingTime, Category = "GameState|Time")
    int32 RemainingTime;

    UFUNCTION()
    virtual void OnRep_RemainingTime();

    /** Server function to set the remaining time. */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameState|Time")
    void SetRemainingTime(int32 NewTime);

    // Delegate for when remaining time changes
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainingTimeChangedDelegate, int32, NewTimeRemaining);
    UPROPERTY(BlueprintAssignable, Category = "GameState|Events")
    FOnRemainingTimeChangedDelegate OnRemainingTimeChangedDelegate;

    // Example: Replicated property for max players allowed in this match
    // UPROPERTY(BlueprintReadOnly, Replicated, Category = "GameState|Rules")
    // int32 MaxPlayers;

protected:
    // Override to add any custom replicated properties
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};