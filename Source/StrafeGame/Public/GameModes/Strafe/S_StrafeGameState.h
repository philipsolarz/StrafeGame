#pragma once

#include "CoreMinimal.h"
#include "GameModes/S_GameStateBase.h" // Inherit from our base GameState
#include "S_StrafeGameState.generated.h"

class AS_StrafeManager; // Manages checkpoints and Strafe logic

UCLASS(Blueprintable, MinimalAPI)
class AS_StrafeGameState : public AS_GameStateBase
{
    GENERATED_BODY()

public:
    AS_StrafeGameState();

    //~ Begin AActor Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    //~ End AActor Interface

    /** The StrafeManager actor for the current Strafe match. Replicated. */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StrafeManager, Category = "StrafeGameState")
    TObjectPtr<AS_StrafeManager> StrafeManager;

    UFUNCTION()
    void OnRep_StrafeManager();

    /** Server function to set the StrafeManager. Called by StrafeGameMode. */
    void SetStrafeManager(AS_StrafeManager* InStrafeManager);

    /** Safe getter for Blueprints to access the Strafe Manager. */
    UFUNCTION(BlueprintPure, Category = "StrafeGameState")
    AS_StrafeManager* GetStrafeManager() const { return StrafeManager; };

    /** Match duration for the current Strafe match (main play phase). Replicated from GameMode settings. */
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "StrafeGameState|Rules")
    int32 MatchDurationSeconds;

    /**
     * Current human-readable name of the match state (e.g., "Warmup", "InProgress", "PostMatch").
     * This can be more specific than the base AGameModeBase::MatchState for UI purposes.
     */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentMatchStateName, Category = "StrafeGameState")
    FName CurrentMatchStateName; // e.g., "Warmup", "RaceInProgress", "RaceFinished"

    UFUNCTION()
    void OnRep_CurrentMatchStateName();

    // Server function to set the custom match state name
    void SetCurrentMatchStateName(FName NewStateName);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentMatchStateNameChanged, FName, NewStateName);
    UPROPERTY(BlueprintAssignable, Category = "StrafeGameState|Events")
    FOnCurrentMatchStateNameChanged OnCurrentMatchStateNameChangedDelegate;

protected:
    // Any other Strafe-specific global replicated state would go here.
    // e.g., overall best time for the current map session, if desired.
};