// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "UI/MainMenu/MenuScreenInterface.h" // Updated include
#include "S_SettingsScreen.generated.h"

class UCommonButtonBase;
class UCommonTabListWidgetBase;
class UCommonAnimatedSwitcher;
class UGameUserSettings;
class UMenuManagerSubsystem;

// Forward declare tab content widgets
// These should ideally be separate UUserWidget C++ classes in UI/MainMenu/Widgets/Settings/
// For simplicity, declared as basic UUserWidget here, but you'd make specific ones.
UCLASS() class STRAFEGAME_API US_SettingsPlayerTab : public UUserWidget { GENERATED_BODY() /* Bind Player Settings here */ };
UCLASS() class STRAFEGAME_API US_SettingsControlsTab : public UUserWidget { GENERATED_BODY() /* Bind Control Settings here */ };
UCLASS() class STRAFEGAME_API US_SettingsGameTab : public UUserWidget { GENERATED_BODY() /* Bind Game Settings here */ };
UCLASS() class STRAFEGAME_API US_SettingsVideoTab : public UUserWidget { GENERATED_BODY() /* Bind Video Settings here */ };
UCLASS() class STRAFEGAME_API US_SettingsAudioTab : public UUserWidget { GENERATED_BODY() /* Bind Audio Settings here */ };


UCLASS()
class STRAFEGAME_API US_SettingsScreen : public UCommonActivatableWidget, public IMenuScreenInterface
{
    GENERATED_BODY()

public:
    US_SettingsScreen();
    virtual void NativeConstruct() override;

    // IMenuScreenInterface
    virtual void SetMenuManager_Implementation(UMenuManagerSubsystem* InMenuManager) override;

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonTabListWidgetBase> TabList_Settings;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonAnimatedSwitcher> Switcher_SettingsTabs;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_ApplySettings;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_RevertSettings;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Back;

    // Bind these in the WBP_SettingsScreen to your actual tab content WBP instances
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<US_SettingsPlayerTab> TabContent_Player;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<US_SettingsControlsTab> TabContent_Controls;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<US_SettingsGameTab> TabContent_Game;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<US_SettingsVideoTab> TabContent_Video;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<US_SettingsAudioTab> TabContent_Audio;

    UFUNCTION()
    void OnApplySettingsClicked();

    UFUNCTION()
    void OnRevertSettingsClicked();

    UFUNCTION()
    void OnBackClicked();

    UFUNCTION()
    void HandleTabSelected(FName TabId);

private:
    UPROPERTY()
    TObjectPtr<UMenuManagerSubsystem> MenuManager;

    UPROPERTY()
    TObjectPtr<UGameUserSettings> GameSettings;

    void LoadSettingsToUI();
    void ApplyUISettingsToGame();
    void RevertUISettings();

    void SetupTabs();
};