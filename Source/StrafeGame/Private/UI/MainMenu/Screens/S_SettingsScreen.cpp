// Source/StrafeGame/Private/UI/MainMenu/Screens/S_SettingsScreen.cpp
#include "UI/MainMenu/Screens/S_SettingsScreen.h"
#include "CommonButtonBase.h"
#include "CommonTabListWidgetBase.h"
#include "CommonAnimatedSwitcher.h"
#include "GameFramework/GameUserSettings.h"
#include "UI/MainMenu/S_MainMenuPlayerController.h" // Changed
#include "Kismet/GameplayStatics.h"
#include "Engine/GameEngine.h"

US_SettingsScreen::US_SettingsScreen()
{
    GameSettings = nullptr;
}

void US_SettingsScreen::NativeConstruct()
{
    Super::NativeConstruct();
    if (GEngine) GameSettings = GEngine->GetGameUserSettings();
    if (Btn_ApplySettings) Btn_ApplySettings->OnClicked().AddUObject(this, &US_SettingsScreen::OnApplySettingsClicked);
    if (Btn_RevertSettings) Btn_RevertSettings->OnClicked().AddUObject(this, &US_SettingsScreen::OnRevertSettingsClicked);
    if (Btn_Back) Btn_Back->OnClicked().AddUObject(this, &US_SettingsScreen::OnBackClicked);
    if (TabList_Settings) TabList_Settings->OnTabSelected.AddDynamic(this, &US_SettingsScreen::HandleTabSelected);
    SetupTabs();
    LoadSettingsToUI();
}

void US_SettingsScreen::SetMainMenuPlayerController_Implementation(AS_MainMenuPlayerController* InPlayerController)
{
    OwningMainMenuPlayerController = InPlayerController;
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
            if (TabList_Settings->GetSelectedTabId() != DefaultTabID || !Switcher_SettingsTabs->GetActiveWidget())
            {
                TabList_Settings->SelectTabByID(DefaultTabID, true);
            }
            else if (TabList_Settings->GetSelectedTabId() != NAME_None && Switcher_SettingsTabs->GetActiveWidget() == nullptr)
            {
                HandleTabSelected(TabList_Settings->GetSelectedTabId());
            }
        }
    }
}

void US_SettingsScreen::LoadSettingsToUI()
{
    if (!GameSettings) return;
    GameSettings->LoadSettings();
    // Example: if (US_SettingsVideoTab* VideoTab = Cast<US_SettingsVideoTab>(TabContent_Video)) { ... }
    UE_LOG(LogTemp, Log, TEXT("Settings Loaded to UI."));
}

void US_SettingsScreen::ApplyUISettingsToGame()
{
    if (!GameSettings) return;
    // Example: if (US_SettingsVideoTab* VideoTab = Cast<US_SettingsVideoTab>(TabContent_Video)) { ... }
    GameSettings->ApplySettings(false);
    UE_LOG(LogTemp, Log, TEXT("UI Settings Applied to GameUserSettings."));
}

void US_SettingsScreen::RevertUISettings()
{
    if (GameSettings) GameSettings->LoadSettings();
    LoadSettingsToUI();
    UE_LOG(LogTemp, Log, TEXT("UI Settings Reverted by reloading from disk."));
}

void US_SettingsScreen::OnApplySettingsClicked()
{
    ApplyUISettingsToGame();
    if (GameSettings) GameSettings->SaveSettings();
    UE_LOG(LogTemp, Log, TEXT("Settings Applied and Saved to disk."));
}

void US_SettingsScreen::OnRevertSettingsClicked()
{
    RevertUISettings();
}

void US_SettingsScreen::OnBackClicked()
{
    // Optionally, ask for confirmation if there are unsaved changes
    RevertUISettings(); // Or handle unsaved changes logic
    if (OwningMainMenuPlayerController.IsValid())
    {
        OwningMainMenuPlayerController->CloseTopmostScreen();
    }
}

void US_SettingsScreen::HandleTabSelected(FName TabId)
{
    UE_LOG(LogTemp, Log, TEXT("Settings Tab selected by ID: %s. CommonUI should handle switcher update."), *TabId.ToString());
}