#include "UI/Widgets/S_MainHUDWidget.h"
#include "Player/S_PlayerController.h"
#include "UI/S_PlayerHUDManager.h"

// Include C++ UUserWidget base classes for child panels
#include "UI/Widgets/S_PlayerStatusWidget.h"
#include "UI/Widgets/S_WeaponInventoryListWidget.h"
#include "UI/Widgets/S_EquippedWeaponDisplayWidget.h"
#include "UI/Widgets/S_GameModeStatusWidget.h"
#include "UI/Widgets/S_AnnouncementsDisplayWidget.h"
#include "UI/Widgets/S_ScreenEffectsWidget.h"

// Include ViewModel classes
#include "UI/ViewModels/S_PlayerHUDViewModel.h"
#include "UI/ViewModels/S_GameModeHUDViewModelBase.h"
#include "UI/ViewModels/S_AnnouncerViewModel.h"
#include "UI/ViewModels/S_ScreenEffectsViewModel.h"

void US_MainHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    AS_PlayerHUDManager* HUDManager = GetPlayerHUDManager();
    if (HUDManager)
    {
        // Fetch and store the root ViewModels
        PlayerViewModel = HUDManager->GetPlayerHUDViewModel();
        GameModeViewModel = HUDManager->GetGameModeHUDViewModel(); // This might change if game mode changes
        AnnouncerViewModel = HUDManager->GetAnnouncerViewModel();
        ScreenEffectsViewModel = HUDManager->GetScreenEffectsViewModel();

        // Initialize child widgets with their respective ViewModels
        InitializeChildWidgetViewModels();

        // Example: If the GameModeViewModel could change dynamically (e.g. joining a match in progress
        // or if AS_PlayerHUDManager::OnMatchStateChanged needs to signal us to re-fetch the GameModeVM).
        // This assumes AS_PlayerHUDManager has a delegate to inform us of this specific change.
        // For now, GameModeViewModel is fetched once. If it's recreated by PlayerHUDManager,
        // this widget would need a way to know and re-fetch/re-assign.
        // A simple way is if AS_PlayerHUDManager itself had an "OnRootViewModelsReadyOrChanged" delegate.
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("US_MainHUDWidget: PlayerHUDManager not found!"));
    }
}

void US_MainHUDWidget::NativeDestruct()
{
    // Unbind any delegates if they were set up here
    Super::NativeDestruct();
}

AS_PlayerHUDManager* US_MainHUDWidget::GetPlayerHUDManager()
{
    if (!CachedPlayerHUDManager && GetOwningPlayer())
    {
        AS_PlayerController* PC = Cast<AS_PlayerController>(GetOwningPlayer());
        if (PC)
        {
            CachedPlayerHUDManager = PC->GetPlayerHUDManagerInstance();
        }
    }
    return CachedPlayerHUDManager;
}

void US_MainHUDWidget::InitializeChildWidgetViewModels()
{
    if (PlayerStatusPanel && PlayerViewModel)
    {
        PlayerStatusPanel->SetViewModel(PlayerViewModel);
    }
    else if (PlayerStatusPanel)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_MainHUDWidget: PlayerStatusPanel is bound, but PlayerViewModel is null."));
    }

    if (WeaponInventoryListPanel && PlayerViewModel)
    {
        WeaponInventoryListPanel->SetPlayerHUDViewModel(PlayerViewModel);
    }
    else if (WeaponInventoryListPanel)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_MainHUDWidget: WeaponInventoryListPanel is bound, but PlayerViewModel is null."));
    }

    if (EquippedWeaponDisplayPanel && PlayerViewModel)
    {
        EquippedWeaponDisplayPanel->SetPlayerHUDViewModel(PlayerViewModel);
    }
    else if (EquippedWeaponDisplayPanel)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_MainHUDWidget: EquippedWeaponDisplayPanel is bound, but PlayerViewModel is null."));
    }

    if (GameModeStatusPanel && GameModeViewModel)
    {
        GameModeStatusPanel->SetViewModel(GameModeViewModel);
    }
    else if (GameModeStatusPanel)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_MainHUDWidget: GameModeStatusPanel is bound, but GameModeViewModel is null."));
    }

    if (AnnouncementsDisplayPanel && AnnouncerViewModel)
    {
        AnnouncementsDisplayPanel->SetViewModel(AnnouncerViewModel);
    }
    else if (AnnouncementsDisplayPanel)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_MainHUDWidget: AnnouncementsDisplayPanel is bound, but AnnouncerViewModel is null."));
    }

    if (ScreenEffectsOverlayPanel && ScreenEffectsViewModel)
    {
        ScreenEffectsOverlayPanel->SetViewModel(ScreenEffectsViewModel);
    }
    else if (ScreenEffectsOverlayPanel)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_MainHUDWidget: ScreenEffectsOverlayPanel is bound, but ScreenEffectsViewModel is null."));
    }
}