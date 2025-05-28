#pragma once

#include "CoreMinimal.h"
#include "GameModes/S_GameStateBase.h" // Inherit from our base GameState
#include "S_ArenaGameState.generated.h"

/**
 * GameState specific to the Arena (Free-For-All) mode.
 * Tracks Arena-specific replicated information like frag limits.
 */
UCLASS(Blueprintable, MinimalAPI)
class AS_ArenaGameState : public AS_GameStateBase
{
    GENERATED_BODY()

public:
    AS_ArenaGameState();

    //~ Begin AActor Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    //~ End AActor Interface

    /** The frag limit for the current Arena match. Replicated from GameMode settings. */
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "ArenaGameState|Rules")
    int32 FragLimit;

    // We already have 'RemainingTime' and 'MatchState' in AS_GameStateBase.
    // If you need to display a specific string for the current state (e.g. "Warmup", "Overtime")
    // you can add a replicated FName/FString property here that GameMode sets.
    // For now, GetMatchState() from base class and RemainingTime should be sufficient.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MatchStateNameOverride, Category = "ArenaGameState")
    FName MatchStateNameOverride;

    UFUNCTION()
    void OnRep_MatchStateNameOverride();

    // Called by Server to set a custom name for the state (e.g. "Warmup", "Overtime")
    void SetMatchStateNameOverride(FName NewName);

    // Delegate for when match state name override changes
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchStateNameOverrideChangedDelegate, FName, NewMatchStateName);
    UPROPERTY(BlueprintAssignable, Category = "GameState|Events")
    FOnMatchStateNameOverrideChangedDelegate OnMatchStateNameOverrideChangedDelegate;


protected:
    // Example: If you wanted to track the current leader for UI display
    // UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CurrentLeader, Category = "ArenaGameState|Score")
    // TObjectPtr<AS_ArenaPlayerState> CurrentLeaderPlayerState;
    // UFUNCTION()
    // virtual void OnRep_CurrentLeader();
    // virtual void UpdateLeader(); // Called on server to check and set leader
};