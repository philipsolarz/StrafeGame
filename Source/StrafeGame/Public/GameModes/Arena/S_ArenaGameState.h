#pragma once

#include "CoreMinimal.h"
#include "GameModes/S_GameStateBase.h"
#include "UI/ViewModels/S_KillfeedItemViewModel.h" // CRITICAL: For FKillfeedEventData definition
#include "S_ArenaGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKillfeedUpdated);

UCLASS(Blueprintable, MinimalAPI)
class AS_ArenaGameState : public AS_GameStateBase
{
    GENERATED_BODY()

public:
    AS_ArenaGameState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "ArenaGameState|Rules")
    int32 FragLimit;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "ArenaGameState|Rules")
    int32 MatchDurationSeconds;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MatchStateNameOverride, Category = "ArenaGameState")
    FName MatchStateNameOverride;

    UFUNCTION()
    void OnRep_MatchStateNameOverride();

    void SetMatchStateNameOverride(FName NewName);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchStateNameOverrideChangedDelegate, FName, NewMatchStateName);
    UPROPERTY(BlueprintAssignable, Category = "GameState|Events")
    FOnMatchStateNameOverrideChangedDelegate OnMatchStateNameOverrideChangedDelegate;

    // Killfeed Data
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RecentKills, Category = "ArenaGameState|Killfeed")
    TArray<FKillfeedEventData> RecentKillsLog; // Ensure this uses FKillfeedEventData

    UFUNCTION()
    void OnRep_RecentKills();

    UPROPERTY(BlueprintAssignable, Category = "ArenaGameState|Events")
    FOnKillfeedUpdated OnKillfeedUpdated;

    // Server function to add a kill to the log
    void AddKillToLog(const FKillfeedEventData& KillInfo); // Ensure this uses FKillfeedEventData
};