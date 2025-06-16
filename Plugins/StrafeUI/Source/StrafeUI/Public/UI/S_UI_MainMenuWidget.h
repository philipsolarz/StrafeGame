// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_MainMenuWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "S_UI_MainMenuWidget.generated.h"

class UCommonButtonBase;

/**
 * @class S_UI_MainMenuWidget
 * @brief The C++ base for the Main Menu screen.
 * Handles user interactions for navigating to other parts of the application.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_MainMenuWidget : public US_UI_BaseScreenWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;

private:
    //~ Button Click Handlers
    UFUNCTION()
    void HandleCreateGameClicked();

    UFUNCTION()
    void HandleFindGameClicked();

    UFUNCTION()
    void HandleLeaderboardsClicked();

    UFUNCTION()
    void HandleReplaysClicked();

    UFUNCTION()
    void HandleSettingsClicked();

    UFUNCTION()
    void HandleQuitClicked();

    //~ UPROPERTY Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Create;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Find;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Leaderboards;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Replays;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Settings;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Quit;
};