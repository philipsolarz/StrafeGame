#pragma once

#include "CoreMinimal.h"
#include "Player/S_PlayerState.h" // Inherit from our base PlayerState
#include "Delegates/DelegateCombinations.h"
#include "S_StrafePlayerState.generated.h"

// Struct to hold race time information (can be moved to a more global header if used elsewhere)
USTRUCT(BlueprintType)
struct FPlayerStrafeRaceTime // Renamed for clarity and to avoid potential global name collisions
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "RaceTime")
    float TotalTime;

    UPROPERTY(BlueprintReadOnly, Category = "RaceTime")
    TArray<float> SplitTimes; // Time at each checkpoint relative to start

    FPlayerStrafeRaceTime() : TotalTime(-1.0f) {} // Initialize total time to an invalid/unset state

    bool IsValid() const { return TotalTime >= 0.0f; }
    void Reset() { TotalTime = -1.0f; SplitTimes.Empty(); }
};

// Delegates for Strafe race events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStrafePlayerRaceStateChangedDelegate, AS_StrafePlayerState*, StrafePlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStrafePlayerNewBestTimeDelegate, const FPlayerStrafeRaceTime&, BestTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStrafePlayerCheckpointHitDelegate, int32, CheckpointIndex, float, TimeAtCheckpoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStrafePlayerFinishedRaceDelegate, float, FinalTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStrafePlayerRaceStartedDelegate);


UCLASS(Blueprintable, Config = Game)
class STRAFEGAME_API AS_StrafePlayerState : public AS_PlayerState
{
    GENERATED_BODY()

public:
    AS_StrafePlayerState();

    //~ Begin AActor Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void Tick(float DeltaSeconds) override;
    //~ End AActor Interface

    //~ Begin APlayerState Interface
    virtual void Reset() override;
    virtual void CopyProperties(APlayerState* PlayerState) override;
    //~ End APlayerState Interface

    // --- Race Control (Server-Side Logic) ---
    /** Starts the race timer and state for the player. Called by server (e.g., RaceManager or StartTrigger). */
    UFUNCTION(BlueprintCallable, Category = "StrafePlayerState|Race", meta = (DisplayName = "Start Race (Server)"))
    void ServerStartRace();

    /** Records that the player has reached a checkpoint. Called by server. */
    UFUNCTION(BlueprintCallable, Category = "StrafePlayerState|Race", meta = (DisplayName = "Reached Checkpoint (Server)"))
    void ServerReachedCheckpoint(int32 CheckpointIndex, int32 TotalCheckpointsInRace);

    /** Records that the player has finished the race. Called by server. */
    UFUNCTION(BlueprintCallable, Category = "StrafePlayerState|Race", meta = (DisplayName = "Finished Race (Server)"))
    void ServerFinishedRace(int32 FinalCheckpointIndex, int32 TotalCheckpointsInRace);

    /** Resets the current race state for the player. Called by server. */
    UFUNCTION(BlueprintCallable, Category = "StrafePlayerState|Race", meta = (DisplayName = "Reset Race State (Server)"))
    void ServerResetRaceState();

    // --- State Queries ---
    UFUNCTION(BlueprintPure, Category = "StrafePlayerState|Race")
    float GetCurrentRaceTime() const { return CurrentRaceTime; }

    UFUNCTION(BlueprintPure, Category = "StrafePlayerState|Race")
    const TArray<float>& GetCurrentSplitTimes() const { return CurrentSplitTimes; }

    UFUNCTION(BlueprintPure, Category = "StrafePlayerState|Race")
    const FPlayerStrafeRaceTime& GetBestRaceTime() const { return BestRaceTime; }

    UFUNCTION(BlueprintPure, Category = "StrafePlayerState|Race")
    int32 GetLastCheckpointReached() const { return LastCheckpointReached; }

    UFUNCTION(BlueprintPure, Category = "StrafePlayerState|Race")
    bool IsRaceInProgress() const { return bIsRaceActiveForPlayer; }

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "StrafePlayerState|Events")
    FOnStrafePlayerRaceStateChangedDelegate OnStrafePlayerRaceStateChangedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "StrafePlayerState|Events")
    FOnStrafePlayerNewBestTimeDelegate OnStrafePlayerNewBestTimeDelegate;

    UPROPERTY(BlueprintAssignable, Category = "StrafePlayerState|Events")
    FOnStrafePlayerCheckpointHitDelegate OnStrafePlayerCheckpointHitDelegate;

    UPROPERTY(BlueprintAssignable, Category = "StrafePlayerState|Events")
    FOnStrafePlayerFinishedRaceDelegate OnStrafePlayerFinishedRaceDelegate;

    UPROPERTY(BlueprintAssignable, Category = "StrafePlayerState|Events")
    FOnStrafePlayerRaceStartedDelegate OnStrafePlayerRaceStartedDelegate;

protected:
    UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentRaceTime)
    float CurrentRaceTime;

    UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentSplitTimes)
    TArray<float> CurrentSplitTimes;

    UPROPERTY(Transient, ReplicatedUsing = OnRep_BestRaceTime)
    FPlayerStrafeRaceTime BestRaceTime;

    UPROPERTY(Transient, ReplicatedUsing = OnRep_LastCheckpointReached)
    int32 LastCheckpointReached;

    UPROPERTY(Transient, ReplicatedUsing = OnRep_IsRaceActiveForPlayer)
    bool bIsRaceActiveForPlayer;

    // --- OnRep Functions ---
    UFUNCTION()
    void OnRep_CurrentRaceTime();
    UFUNCTION()
    void OnRep_CurrentSplitTimes();
    UFUNCTION()
    void OnRep_BestRaceTime();
    UFUNCTION()
    void OnRep_LastCheckpointReached();
    UFUNCTION()
    void OnRep_IsRaceActiveForPlayer();

    /** Called internally to broadcast generic state change and manage tick enabling. */
    void BroadcastStrafeStateUpdate();
};