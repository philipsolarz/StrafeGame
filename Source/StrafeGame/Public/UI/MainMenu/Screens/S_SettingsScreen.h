// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "UI/MainMenu/MenuScreenInterface.h" 
#include "S_SettingsScreen.generated.h"

class UCommonButtonBase;
class UCommonTabListWidgetBase;
class UCommonAnimatedSwitcher;
class UGameUserSettings;
class AS_MainMenuPlayerController; // Changed

UCLASS() class STRAFEGAME_API US_SettingsPlayerTab : public UUserWidget { GENERATED_BODY() };
UCLASS() class STRAFEGAME_API US_SettingsControlsTab : public UUserWidget { GENERATED_BODY() };
UCLASS() class STRAFEGAME_API US_SettingsGameTab : public UUserWidget { GENERATED_BODY() };
UCLASS() class STRAFEGAME_API US_SettingsVideoTab : public UUserWidget { GENERATED_BODY() };
UCLASS() class STRAFEGAME_API US_SettingsAudioTab : public UUserWidget { GENERATED_BODY() };

UCLASS()
class STRAFEGAME_API US_SettingsScreen : public UCommonActivatableWidget, public IMenuScreenInterface
{
    GENERATED_BODY()

public:
    US_SettingsScreen();
    virtual void NativeConstruct() override;
    virtual void SetMainMenuPlayerController_Implementation(AS_MainMenuPlayerController* InPlayerController) override; // Updated

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

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<US_SettingsPlayerTab> TabContent_Player;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<US_SettingsControlsTab> TabContent_Controls;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<US_SettingsGameTab> TabContent_Game;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<US_SettingsVideoTab> TabContent_Video;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<US_SettingsAudioTab> TabContent_Audio;

    UFUNCTION() void OnApplySettingsClicked();
    UFUNCTION() void OnRevertSettingsClicked();
    UFUNCTION() void OnBackClicked();
    UFUNCTION() void HandleTabSelected(FName TabId);

private:
    UPROPERTY()
    TWeakObjectPtr<AS_MainMenuPlayerController> OwningMainMenuPlayerController; // Changed

    UPROPERTY()
    TObjectPtr<UGameUserSettings> GameSettings;

    void LoadSettingsToUI();
    void ApplyUISettingsToGame();
    void RevertUISettings();
    void SetupTabs();
};