#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S_GameModeStatusWidget.generated.h"

class UWidgetSwitcher;
class US_GameModeHUDViewModelBase;
class US_ArenaStatusWidget;    // C++ base for WBP_ArenaStatus
class US_StrafeStatusWidget;   // C++ base for WBP_StrafeStatus

UCLASS()
class STRAFEGAME_API US_GameModeStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "GameMode Status Widget")
    void SetViewModel(US_GameModeHUDViewModelBase* InViewModel);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModel")
    TObjectPtr<US_GameModeHUDViewModelBase> GameModeViewModel;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UWidgetSwitcher> GameModeSpecificSwitcher;

    // Widget Classes to spawn into the switcher
    UPROPERTY(EditDefaultsOnly, Category = "WidgetClasses")
    TSubclassOf<US_ArenaStatusWidget> ArenaStatusWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "WidgetClasses")
    TSubclassOf<US_StrafeStatusWidget> StrafeStatusWidgetClass;

    // Instances
    UPROPERTY()
    TObjectPtr<US_ArenaStatusWidget> ArenaStatusWidgetInstance;
    UPROPERTY()
    TObjectPtr<US_StrafeStatusWidget> StrafeStatusWidgetInstance;

    UFUNCTION()
    virtual void HandleViewModelUpdated();

    virtual void UpdateActiveGameModeWidget();

private:
    UUserWidget* GetOrCreateGameModeSpecificWidget(TSubclassOf<UUserWidget> WidgetClass, int32& OutWidgetIndexInSwitcher);
};