// Source/StrafeGame/Private/UI/MainMenu/Screens/S_SettingsScreen.cpp
#include "UI/MainMenu/Screens/S_SettingsScreen.h"
#include "CommonButtonBase.h"
#include "CommonTabListWidgetBase.h"
#include "CommonAnimatedSwitcher.h"
#include "GameFramework/GameUserSettings.h" // Added include
#include "UI/MainMenu/MenuManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

US_SettingsScreen::US_SettingsScreen()
{
    GameSettings = nullptr;
}

void US_SettingsScreen::NativeConstruct()
{
    Super::NativeConstruct();

    GameSettings = GEngine->GetGameUserSettings(); // Correct way to get GameUserSettings

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
        // Corrected: Removed extra parentheses from OnTabSelected
        TabList_Settings->OnTabSelected.AddUObject(this, &US_SettingsScreen::HandleTabSelected);
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

    if (TabList_Settings->GetTabCount() > 0)
    {
        FName DefaultTabID = TabList_Settings->GetTabIdAtIndex(0);
        if (DefaultTabID != NAME_None)
        {
            // Check if a tab is already selected, possibly by UMG default
            if (TabList_Settings->GetSelectedTabID() != DefaultTabID || !Switcher_SettingsTabs->GetActiveWidget())
            {
                TabList_Settings->SetSelectedTabID(DefaultTabID, true); // true to broadcast, CommonUI should handle switcher
                // HandleTabSelected(DefaultTabID); // No longer needed if SetSelectedTabID broadcasts and switcher is linked
            }
            else if (TabList_Settings->GetSelectedTabID() != NAME_None && Switcher_SettingsTabs->GetActiveWidget() == nullptr)
            {
                // If a tab is selected but switcher has no active widget, force sync.
                // This case might happen if UMG sets a default selected tab but the switcher linking needs an explicit kick.
                HandleTabSelected(TabList_Settings->GetSelectedTabID());
            }
        }
    }
}


void US_SettingsScreen::LoadSettingsToUI()
{
    if (!GameSettings) return;

    GameSettings->LoadSettings(); // Load from disk

    // Example for Video Tab (assuming TabContent_Video is a US_SettingsVideoTab with relevant controls)
    // if (US_SettingsVideoTab* VideoTab = Cast<US_SettingsVideoTab>(TabContent_Video))
    // {
    //     // VideoTab->DisplayResolutionComboBox->SetSelectedOption(GameSettings->GetScreenResolution().ToString());
    //     // VideoTab->FullscreenCheckBox->SetIsChecked(GameSettings->GetFullscreenMode() == EWindowMode::Fullscreen);
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
    // }

    GameSettings->ApplySettings(false); // false to not apply to system immediately, just update internal state. SaveSettings will apply.
    UE_LOG(LogTemp, Log, TEXT("UI Settings Applied to GameUserSettings."));
}

void US_SettingsScreen::RevertUISettings()
{
    // Reload settings from disk to effectively revert any UI changes not yet applied/saved
    if (GameSettings)
    {
        GameSettings->LoadSettings(); // Reload from disk
    }
    LoadSettingsToUI(); // Then re-populate UI with these reloaded settings
    UE_LOG(LogTemp, Log, TEXT("UI Settings Reverted by reloading from disk."));
}

void US_SettingsScreen::OnApplySettingsClicked()
{
    ApplyUISettingsToGame();
    if (GameSettings) GameSettings->SaveSettings(); // This applies and saves to disk
    UE_LOG(LogTemp, Log, TEXT("Settings Applied and Saved to disk."));
}

void US_SettingsScreen::OnRevertSettingsClicked()
{
    RevertUISettings();
}

void US_SettingsScreen::OnBackClicked()
{
    // Optionally ask for confirmation if there are unsaved changes
    RevertUISettings(); // Revert any UI changes not applied
    if (MenuManager)
    {
        MenuManager->CloseTopmostScreen();
    }
}

void US_SettingsScreen::HandleTabSelected(FName TabId)
{
    // CommonUI and UCommonTabListWidgetBase with SetLinkedSwitcher should handle
    // activating the correct widget in the UCommonAnimatedSwitcher.
    // The TabId is the "Tab ID" property set on the UCommonButton (or similar tab widget)
    // within the TabList. The Switcher_SettingsTabs must have child widgets whose names
    // or indices correspond to these Tab IDs for the linking to work automatically.

    UE_LOG(LogTemp, Log, TEXT("Settings Tab selected by ID: %s. CommonUI should handle switcher update if linked correctly."), *TabId.ToString());

    // Manual switching as a fallback or for explicit control if needed:
    // if (!Switcher_SettingsTabs) return;
    // if (TabContent_Video && TabId == FName(TEXT("VideoTabID"))) Switcher_SettingsTabs->SetActiveWidget(TabContent_Video);
    // else if (TabContent_Audio && TabId == FName(TEXT("AudioTabID"))) Switcher_SettingsTabs->SetActiveWidget(TabContent_Audio);
    // ... and so on for other tabs
}