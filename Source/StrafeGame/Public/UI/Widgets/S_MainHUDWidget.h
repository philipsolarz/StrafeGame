#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S_MainHUDWidget.generated.h"

// Forward declare C++ UUserWidget base classes for child panels
class US_PlayerStatusWidget;
class US_WeaponInventoryListWidget;
class US_EquippedWeaponDisplayWidget;
class US_GameModeStatusWidget;
class US_AnnouncementsDisplayWidget;
class US_ScreenEffectsWidget;

// Forward declare ViewModel classes
class AS_PlayerHUDManager;
class US_PlayerHUDViewModel;
class US_GameModeHUDViewModelBase;
class US_AnnouncerViewModel;
class US_ScreenEffectsViewModel;

UCLASS()
class STRAFEGAME_API US_MainHUDWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // --- ViewModels ---
    // These will be fetched from AS_PlayerHUDManager
    UPROPERTY(BlueprintReadOnly, Category = "ViewModels")
    TObjectPtr<US_PlayerHUDViewModel> PlayerViewModel;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModels")
    TObjectPtr<US_GameModeHUDViewModelBase> GameModeViewModel;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModels")
    TObjectPtr<US_AnnouncerViewModel> AnnouncerViewModel;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModels")
    TObjectPtr<US_ScreenEffectsViewModel> ScreenEffectsViewModel;

    // --- Bound Child Widget Panels ---
    // Ensure instances of WBP_PlayerStatus (derived from US_PlayerStatusWidget) etc.
    // are named these in the WBP_MainHUD designer.
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<US_PlayerStatusWidget> PlayerStatusPanel;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<US_WeaponInventoryListWidget> WeaponInventoryListPanel;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<US_EquippedWeaponDisplayWidget> EquippedWeaponDisplayPanel;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<US_GameModeStatusWidget> GameModeStatusPanel; // This is the container for Arena/Strafe status

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<US_AnnouncementsDisplayWidget> AnnouncementsDisplayPanel;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<US_ScreenEffectsWidget> ScreenEffectsOverlayPanel;

private:
    UPROPERTY()
    TObjectPtr<AS_PlayerHUDManager> CachedPlayerHUDManager;

    AS_PlayerHUDManager* GetPlayerHUDManager();
    void InitializeChildWidgetViewModels();

    // Handler for when the GameModeViewModel itself might change (e.g. game mode switch if dynamic)
    // This would be bound if PlayerHUDManager had such a delegate, or if we directly react to GameState changes.
    // For now, GameModeViewModel is set once during NativeConstruct from PlayerHUDManager.
    // If AS_PlayerHUDManager::OnMatchStateChanged() updates its exposed GameModeViewModel,
    // we might need a way to re-fetch it or have PlayerHUDManager explicitly update us.
    // For simplicity, assuming GameModeViewModel from PlayerHUDManager is the one to use.
};