#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameModes/Strafe/S_StrafePlayerState.h" // For FPlayerStrafeRaceTime
#include "S_StrafeManager.generated.h"

// Forward Declarations
class AS_CheckpointTrigger;
class AS_Character;
class APlayerState; // Base APlayerState for wider compatibility if needed, though AS_StrafePlayerState is primary
class AS_StrafePlayerState;

USTRUCT(BlueprintType)
struct FPlayerScoreboardEntry_Strafe
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "StrafeManager|Scoreboard")
    FString PlayerName;

    UPROPERTY(BlueprintReadOnly, Category = "StrafeManager|Scoreboard")
    FPlayerStrafeRaceTime BestTime; // Using the struct from AS_StrafePlayerState

    // Store the PlayerState to potentially retrieve more info or ensure correct player association
    UPROPERTY(Transient) // Not replicated directly as part of this struct, scoreboard array is replicated
        TWeakObjectPtr<APlayerState> PlayerStateRef;

    FPlayerScoreboardEntry_Strafe()
    {
        PlayerName = TEXT("N/A");
        BestTime.TotalTime = FLT_MAX; // Initialize with a very large time
    }

    // Comparison for sorting
    bool operator<(const FPlayerScoreboardEntry_Strafe& Other) const
    {
        return BestTime.TotalTime < Other.BestTime.TotalTime;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStrafeScoreboardUpdatedDelegate);

UCLASS(Blueprintable, Config = Game) // Removed NotPlaceable to allow placement if needed
class STRAFEGAME_API AS_StrafeManager : public AActor
{
    GENERATED_BODY()

public:
    AS_StrafeManager();

    //~ Begin AActor Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    //~ End AActor Interface

    /**
     * Scans the level for all AS_CheckpointTrigger actors and registers them.
     * Sorts them by CheckpointOrder.
     * Identifies Start and Finish lines.
     * Should be called by the GameMode after level loading or if checkpoints can change dynamically.
     */
    UFUNCTION(BlueprintCallable, Category = "StrafeManager|Setup")
    void RefreshAndInitializeCheckpoints();

    /**
     * Called by an AS_CheckpointTrigger when a player overlaps it.
     * This is the primary entry point for player race progress.
     * Server-authoritative.
     * @param Checkpoint The checkpoint actor that was reached.
     * @param PlayerCharacter The character that reached the checkpoint.
     */
    UFUNCTION() // Needs to be UFUNCTION to bind to delegate
        virtual void HandleCheckpointReached(AS_CheckpointTrigger* Checkpoint, AS_Character* PlayerCharacter);

    /**
     * Updates a player's entry in the scoreboard or adds a new one.
     * Typically called when a player finishes a race or when they join.
     * Server-authoritative.
     * @param PlayerState The PlayerState of the player to update. Must be AS_StrafePlayerState.
     */
    UFUNCTION(BlueprintCallable, Category = "StrafeManager|Scoreboard")
    void UpdatePlayerInScoreboard(AS_PlayerState* PlayerState);


    UFUNCTION(BlueprintPure, Category = "StrafeManager|Scoreboard")
    const TArray<FPlayerScoreboardEntry_Strafe>& GetScoreboard() const { return Scoreboard; }

    UPROPERTY(BlueprintAssignable, Category = "StrafeManager|Events")
    FOnStrafeScoreboardUpdatedDelegate OnScoreboardUpdatedDelegate;

    UFUNCTION(BlueprintPure, Category = "StrafeManager|Setup")
    AS_CheckpointTrigger* GetStartLine() const { return StartLine; }

    UFUNCTION(BlueprintPure, Category = "StrafeManager|Setup")
    AS_CheckpointTrigger* GetFinishLine() const { return FinishLine; }

    UFUNCTION(BlueprintPure, Category = "StrafeManager|Setup")
    int32 GetTotalCheckpointsForLap() const { return TotalCheckpointsForFullLap; }


protected:
    /**
     * All registered checkpoints in the race, sorted by their CheckpointOrder.
     * Replicated so clients can potentially draw paths or get info.
     */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "StrafeManager|Setup")
    TArray<TObjectPtr<AS_CheckpointTrigger>> AllCheckpointsInOrder;

    /** The current scoreboard. Replicated. */
    UPROPERTY(ReplicatedUsing = OnRep_Scoreboard, BlueprintReadOnly, Category = "StrafeManager|Scoreboard")
    TArray<FPlayerScoreboardEntry_Strafe> Scoreboard;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "StrafeManager|Setup") // Not replicated, server uses it. Clients can get via getter.
        TObjectPtr<AS_CheckpointTrigger> StartLine;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "StrafeManager|Setup")
    TObjectPtr<AS_CheckpointTrigger> FinishLine;

    /** Includes start and finish line checkpoints. Set after RefreshAndInitializeCheckpoints. */
    int32 TotalCheckpointsForFullLap;

    UFUNCTION()
    virtual void OnRep_Scoreboard();

    /** Sorts the AllCheckpointsInOrder array by CheckpointOrder. */
    void SortCheckpoints();

    /** After sorting, identifies StartLine, FinishLine, and TotalCheckpointsForFullLap. */
    void FinalizeCheckpointSetup();

    /** Registers a single checkpoint. Called by RefreshAndInitializeCheckpoints. */
    void RegisterCheckpoint(AS_CheckpointTrigger* Checkpoint);
};