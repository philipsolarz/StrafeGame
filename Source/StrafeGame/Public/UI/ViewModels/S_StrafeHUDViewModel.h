#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/S_GameModeHUDViewModelBase.h"
#include "GameModes/Strafe/S_StrafePlayerState.h" 
#include "S_StrafeHUDViewModel.generated.h"

class AS_StrafeGameState;
class AS_StrafePlayerState;

UCLASS()
class STRAFEGAME_API US_StrafeHUDViewModel : public US_GameModeHUDViewModelBase
{
    GENERATED_BODY()

public:
    virtual void Initialize(AS_PlayerController* InOwningPlayerController) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintReadOnly, Category = "StrafeViewModel")
    float CurrentRaceTime;

    UPROPERTY(BlueprintReadOnly, Category = "StrafeViewModel")
    FPlayerStrafeRaceTime BestRaceTime;

    UPROPERTY(BlueprintReadOnly, Category = "StrafeViewModel")
    TArray<float> CurrentSplitTimes;

    UPROPERTY(BlueprintReadOnly, Category = "StrafeViewModel")
    TArray<float> SplitDeltas;

    UPROPERTY(BlueprintReadOnly, Category = "StrafeViewModel")
    int32 CurrentCheckpoint;

    UPROPERTY(BlueprintReadOnly, Category = "StrafeViewModel")
    int32 TotalCheckpoints;

    UPROPERTY(BlueprintReadOnly, Category = "StrafeViewModel")
    bool bIsRaceActive;

protected:
    TWeakObjectPtr<AS_StrafeGameState> StrafeGameState;
    TWeakObjectPtr<AS_StrafePlayerState> LocalStrafePlayerState;

    FTimerHandle UpdateTimerHandle;

    virtual void RefreshGameModeData() override;
    void UpdateStrafeSpecificData();
    void UpdateRaceTime();

    UFUNCTION()
    void HandleStrafeRaceStateChanged(AS_StrafePlayerState* InPlayerState);
};