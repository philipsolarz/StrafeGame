// Plugins/StrafeUI/Source/StrafeUI/Public/S_UI_Settings.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "S_UI_InputController.h"
#include "S_UI_ModalStack.h"
#include "Data/S_UI_ScreenDataAsset.h"
#include "UI/S_UI_RootWidget.h"
#include "UI/S_UI_MainMenuWidget.h"
#include "UI/S_UI_ModalWidget.h"
#include "InputAction.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "S_UI_Settings.generated.h"

// Forward declarations
class US_UI_SettingsTabBase;
class UCommonButtonBase;

/**
 * Defines a game mode and the list of maps compatible with it.
 */
USTRUCT(BlueprintType)
struct FStrafeGameModeInfo
{
    GENERATED_BODY()

    /** The name displayed in the UI for this game mode. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Mode")
    FText DisplayName;

    /** A reference to the Game Mode's Blueprint asset. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Mode")
    TSoftClassPtr<AGameModeBase> GameModeClass;

    /** The list of maps that can be played with this game mode. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Mode")
    TArray<TSoftObjectPtr<UWorld>> CompatibleMaps;
};

/**
 * Defines all configurable settings for the StrafeUI plugin.
 */
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Strafe UI"))
class STRAFEUI_API US_UI_Settings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    //~ Begin Core Settings
    /** The widget class to use for the root of the UI. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core")
    TSoftClassPtr<US_UI_RootWidget> RootWidgetClass;

    /** The widget class to use for the persistent Main Menu sidebar. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core")
    TSoftClassPtr<US_UI_MainMenuWidget> MainMenuWidgetClass;

    /** Data asset containing the mapping of content screen IDs to widget classes. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core")
    TSoftObjectPtr<US_UI_ScreenDataAsset> ScreenMapDataAsset;

    /** The widget class to use for creating modals. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core")
    TSoftClassPtr<US_UI_ModalWidget> ModalWidgetClass;

    /** The class to use for the UI Input Controller. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core")
    TSoftClassPtr<US_UI_InputController> InputControllerClass;

    /** The class to use for the UI Modal Stack. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core")
    TSoftClassPtr<US_UI_ModalStack> ModalStackClass;

    /** The button class to use for tabs. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core")
    TSoftClassPtr<UCommonButtonBase> TabButtonClass;
    //~ End Core Settings

    //~ Begin Create Game Screen Settings
    /** The list of all game modes and their compatible maps available to be created. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Create Game Screen")
    TArray<FStrafeGameModeInfo> AvailableGameModes;
    //~ End Create Game Screen Settings

    //~ Begin Settings Tab Classes
    /** The widget class for the Audio settings tab. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Settings Tabs")
    TSoftClassPtr<US_UI_SettingsTabBase> AudioSettingsTabClass;

    /** The widget class for the Video settings tab. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Settings Tabs")
    TSoftClassPtr<US_UI_SettingsTabBase> VideoSettingsTabClass;

    /** The widget class for the Controls settings tab. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Settings Tabs")
    TSoftClassPtr<US_UI_SettingsTabBase> ControlsSettingsTabClass;

    /** The widget class for the Gameplay settings tab. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Settings Tabs")
    TSoftClassPtr<US_UI_SettingsTabBase> GameplaySettingsTabClass;

    /** The widget class for the Player settings tab. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Settings Tabs")
    TSoftClassPtr<US_UI_SettingsTabBase> PlayerSettingsTabClass;
    //~ End Settings Tab Classes

    //~ Begin Input Settings
    /** Input Action for UI Navigation (e.g., Gamepad D-pad, WASD). */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Input")
    TSoftObjectPtr<UInputAction> NavigateAction;

    /** Input Action for confirming a selection. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Input")
    TSoftObjectPtr<UInputAction> AcceptAction;

    /** Input Action for going back or canceling. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Input")
    TSoftObjectPtr<UInputAction> BackAction;
    //~ End Input Settings
};