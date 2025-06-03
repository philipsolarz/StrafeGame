// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/MainMenu/Screens/S_SettingsScreen.h"
#include "CommonButtonBase.h"
#include "CommonTabListWidgetBase.h"
#include "Components/WidgetSwitcher.h" // Keep for casting from UWidget if needed, but switcher itself is CommonAnimatedSwitcher
#include "CommonAnimatedSwitcher.h" // Added include
#include "GameFramework/GameUserSettings.h"
#include "UI/MainMenu/MenuManagerSubsystem.h"
#include "Kismet/GameplayStatics.h" // Ensure this is included for GetGameUserSettings

US_SettingsScreen::US_SettingsScreen()
{
    GameSettings = nullptr;
}

void US_SettingsScreen::NativeConstruct()
{
    Super::NativeConstruct();

    GameSettings = UGameplayStatics::GetGameUserSettings(); // This function definitely exists

    if (Btn_ApplySettings)
    {
        Btn_ApplySettings->OnClicked().AddUObject(this, &US_SettingsScreen::OnApplySettingsClicked);
    }
    if (Btn_RevertSettings)
    {
        Btn_RevertSettings->OnClicked().AddUObject(this, &US_SettingsScreen::OnRevertSettingsClicked);
    }
    if (Btn_Back)
    {
        Btn_Back->OnClicked().AddUObject(this, &US_SettingsScreen::OnBackClicked);
    }
    if (TabList_Settings)
    {
        // Corrected: UCommonTabListWidgetBase::OnTabSelected is the C++ delegate
        // It's FOnTabSelected type: DECLARE_MULTICAST_DELEGATE_OneParam(FOnTabSelected, FName /* TabId */);
        // AddUObject is appropriate for non-UFUNCTION handlers. If HandleTabSelected is UFUNCTION, AddDynamic is fine.
        TabList_Settings->OnTabSelected().AddUObject(this, &US_SettingsScreen::HandleTabSelected);
    }

    SetupTabs();
    LoadSettingsToUI();
}

void US_SettingsScreen::SetMenuManager_Implementation(UMenuManagerSubsystem* InMenuManager)
{
    MenuManager = InMenuManager;
}

void US_SettingsScreen::SetupTabs()
{
    if (!TabList_Settings || !Switcher_SettingsTabs) return;

    TabList_Settings->SetLinkedSwitcher(Switcher_SettingsTabs);

    // Ensure tabs are registered if not done in BP, or that Tab IDs are correctly set on Tab Buttons.
    // For this setup, we assume Tab Buttons are created in UMG with appropriate Tab IDs.
    // Example: A Tab Button in UMG with "Tab Button ID" set to "VideoTabID" will be handled by CommonUI
    // to show the widget in the Switcher_SettingsTabs that is either at the same index
    // or that you explicitly map (less common for basic setups).

    if (TabList_Settings->GetTabCount() > 0)
    {
        FName DefaultTabID = TabList_Settings->GetTabIdAtIndex(0);
        if (DefaultTabID != NAME_None && TabList_Settings->GetSelectedTabID() != DefaultTabID) // Check if already selected
        {
            TabList_Settings->SetSelectedTabID(DefaultTabID, false); // false to not broadcast, let HandleTabSelected do it or it'll fire twice.
            HandleTabSelected(DefaultTabID); // Manually call to ensure switcher updates
        }
        else if (TabList_Settings->GetSelectedTabID() != NAME_None) // If a tab is already selected by default in UMG
        {
            HandleTabSelected(TabList_Settings->GetSelectedTabID()); // Ensure switcher is synced
        }
    }
}


void US_SettingsScreen::LoadSettingsToUI()
{
    if (!GameSettings) return;

    GameSettings->LoadSettings();

    // Example for Video Tab (assuming TabContent_Video is a US_SettingsVideoTab with relevant controls)
    // if (US_SettingsVideoTab* VideoTab = Cast<US_SettingsVideoTab>(TabContent_Video))
    // {
    //     // VideoTab->DisplayResolutionComboBox->SetSelectedOption(GameSettings->GetScreenResolution().ToString());
    //     // VideoTab->FullscreenCheckBox->SetIsChecked(GameSettings->GetFullscreenMode() == EWindowMode::Fullscreen);
    //     // ... load other settings into their respective UI elements ...
    // }

    UE_LOG(LogTemp, Log, TEXT("Settings Loaded to UI."));
}

void US_SettingsScreen::ApplyUISettingsToGame()
{
    if (!GameSettings) return;

    // Example for Video Tab
    // if (US_SettingsVideoTab* VideoTab = Cast<US_SettingsVideoTab>(TabContent_Video))
    // {
    //     // FIntPoint SelectedResolution = ... parse from VideoTab->DisplayResolutionComboBox ...
    //     // GameSettings->SetScreenResolution(SelectedResolution);
    //     // EWindowMode::Type NewWindowMode = VideoTab->FullscreenCheckBox->IsChecked() ? EWindowMode::Fullscreen : EWindowMode::Windowed;
    //     // GameSettings->SetFullscreenMode(NewWindowMode);
    //     // ... apply other settings ...
    // }

    GameSettings->ApplySettings(false);
    UE_LOG(LogTemp, Log, TEXT("UI Settings Applied to GameUserSettings."));
}

void US_SettingsScreen::RevertUISettings()
{
    LoadSettingsToUI();
    UE_LOG(LogTemp, Log, TEXT("UI Settings Reverted."));
}

void US_SettingsScreen::OnApplySettingsClicked()
{
    ApplyUISettingsToGame();
    if (GameSettings) GameSettings->SaveSettings();
    UE_LOG(LogTemp, Log, TEXT("Settings Saved to disk."));
}

void US_SettingsScreen::OnRevertSettingsClicked()
{
    RevertUISettings();
}

void US_SettingsScreen::OnBackClicked()
{
    RevertUISettings();
    if (MenuManager)
    {
        MenuManager->CloseTopmostScreen();
    }
}

void US_SettingsScreen::HandleTabSelected(FName TabId)
{
    // With SetLinkedSwitcher, CommonUI should handle activating the correct widget in the Switcher.
    // The TabId here is the "Tab ID" property you set on the UCommonButton (or similar tab widget) within the TabList.
    // The UCommonAnimatedSwitcher then needs to have child widgets whose names or some other identifier
    // can be matched by the TabList to activate the correct one. Often, index-based is default if not customized.

    // You typically don't need to manually call Switcher_SettingsTabs->SetActiveWidget here if linked correctly.
    // However, if you needed to do it manually:
    /*
    if (!Switcher_SettingsTabs) return;
    if (TabId == FName(TEXT("PlayerTabID")) && TabContent_Player) Switcher_SettingsTabs->SetActiveWidget(TabContent_Player);
    else if (TabId == FName(TEXT("ControlsTabID")) && TabContent_Controls) Switcher_SettingsTabs->SetActiveWidget(TabContent_Controls);
    else if (TabId == FName(TEXT("GameTabID")) && TabContent_Game) Switcher_SettingsTabs->SetActiveWidget(TabContent_Game);
    else if (TabId == FName(TEXT("VideoTabID")) && TabContent_Video) Switcher_SettingsTabs->SetActiveWidget(TabContent_Video);
    else if (TabId == FName(TEXT("AudioTabID")) && TabContent_Audio) Switcher_SettingsTabs->SetActiveWidget(TabContent_Audio);
    */
    UE_LOG(LogTemp, Log, TEXT("Settings Tab selected by ID: %s. CommonUI should handle switcher."), *TabId.ToString());
}