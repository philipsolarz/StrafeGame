// Source/StrafeGame/Public/UI/ViewModels/S_GameModeHUDViewModelBase.h
#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/S_ViewModelBase.h"
#include "S_GameModeHUDViewModelBase.generated.h"

class AS_GameStateBase;

UCLASS(Abstract)
class STRAFEGAME_API US_GameModeHUDViewModelBase : public US_ViewModelBase
{
    GENERATED_BODY()

public:
    virtual void Initialize(AS_PlayerController* InOwningPlayerController) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintReadOnly, Category = "GameModeViewModel")
    FName CurrentMatchStateName;

    UPROPERTY(BlueprintReadOnly, Category = "GameModeViewModel")
    int32 RemainingMatchTimeSeconds;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameModeViewModelUpdated);
    UPROPERTY(BlueprintAssignable, Category = "GameModeViewModel|Events")
    FOnGameModeViewModelUpdated OnGameModeViewModelUpdated;

protected:
    TWeakObjectPtr<AS_GameStateBase> GameStateBase;

    // FDelegateHandle MatchStateChangedHandle; // Not needed for AddDynamic
    // FDelegateHandle RemainingTimeChangedHandle; // Not needed for AddDynamic

    UFUNCTION() // Mark as UFUNCTION for AddDynamic
        virtual void HandleMatchStateChanged(FName NewMatchState);

    UFUNCTION() // Mark as UFUNCTION for AddDynamic
        virtual void HandleRemainingTimeChanged(int32 NewTime);

    virtual void UpdateMatchStateName();
    virtual void UpdateRemainingTime();
    virtual void RefreshGameModeData();
};