#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "S_UI_PauseMenuWidget.generated.h"

class UCommonButtonBase;
class US_UI_InGameActionsWidget;

/**
 * The main pause menu widget that appears when the player pauses the game.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_PauseMenuWidget : public US_UI_BaseScreenWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;

private:
    UFUNCTION()
    void HandleResumeClicked();

    UFUNCTION()
    void HandleSettingsClicked();

    UFUNCTION()
    void HandleInGameActionsClicked();

    UFUNCTION()
    void HandleExitToMenuClicked();

    UFUNCTION()
    void HandleExitToDesktopClicked();

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Resume;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Settings;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_InGameActions;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_ExitToMenu;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_ExitToDesktop;

    UPROPERTY(EditDefaultsOnly, Category = "Pause Menu")
    TSubclassOf<US_UI_InGameActionsWidget> InGameActionsWidgetClass;

    UPROPERTY()
    TObjectPtr<US_UI_InGameActionsWidget> InGameActionsWidgetInstance;
};