#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "S_PlayerHUDManager.generated.h"

class UUserWidget;
class AS_PlayerController;
class US_PlayerHUDViewModel; // Added
class US_GameModeHUDViewModelBase; // Added
class US_AnnouncerViewModel; // Added
class US_ScreenEffectsViewModel; // Added

// Forward declare specific game mode VM types
class US_ArenaHUDViewModel;
class US_StrafeHUDViewModel;


UCLASS()
class STRAFEGAME_API AS_PlayerHUDManager : public AActor
{
    GENERATED_BODY()

public:
    AS_PlayerHUDManager();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void Initialize(AS_PlayerController* OwningPlayerController);

    UFUNCTION(BlueprintPure, Category = "HUDManager")
    AS_PlayerController* GetOwningStrafePlayerController() const;

    // Expose ViewModels to Blueprints (e.g., for the root WBP_MainHUD to access)
    UFUNCTION(BlueprintPure, Category = "HUDManager|ViewModels")
    US_PlayerHUDViewModel* GetPlayerHUDViewModel() const { return PlayerHUDViewModel; }

    UFUNCTION(BlueprintPure, Category = "HUDManager|ViewModels")
    US_GameModeHUDViewModelBase* GetGameModeHUDViewModel() const { return GameModeHUDViewModel; }

    UFUNCTION(BlueprintPure, Category = "HUDManager|ViewModels")
    US_AnnouncerViewModel* GetAnnouncerViewModel() const { return AnnouncerViewModel; }

    UFUNCTION(BlueprintPure, Category = "HUDManager|ViewModels")
    US_ScreenEffectsViewModel* GetScreenEffectsViewModel() const { return ScreenEffectsViewModel; }

    // Function to trigger announcements from game systems
    UFUNCTION(BlueprintCallable, Category = "HUDManager|Announcer")
    void ShowGameAnnouncement(const FText& Message, float Duration = 3.0f);

protected:
    UPROPERTY(Transient)
    TObjectPtr<AS_PlayerController> OwningStrafePC;

    UPROPERTY(BlueprintReadOnly, Category = "HUDManager|Widgets", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UUserWidget> MainHUDWidget;

    UPROPERTY(EditDefaultsOnly, Category = "HUDManager|Widgets", Meta = (DisplayName = "Main HUD Widget Class"))
    TSubclassOf<UUserWidget> MainHUDWidgetClass;

    // ViewModels - owned by the HUD Manager
    UPROPERTY(BlueprintReadOnly, Category = "HUDManager|ViewModels", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<US_PlayerHUDViewModel> PlayerHUDViewModel;

    UPROPERTY(BlueprintReadOnly, Category = "HUDManager|ViewModels", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<US_GameModeHUDViewModelBase> GameModeHUDViewModel; // Will hold Arena or Strafe VM

    UPROPERTY(BlueprintReadOnly, Category = "HUDManager|ViewModels", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<US_AnnouncerViewModel> AnnouncerViewModel;

    UPROPERTY(BlueprintReadOnly, Category = "HUDManager|ViewModels", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<US_ScreenEffectsViewModel> ScreenEffectsViewModel;

    // Specific GameMode ViewModel classes (to be set in BP if this manager is subclassed, or detected at runtime)
    UPROPERTY(EditDefaultsOnly, Category = "HUDManager|ViewModels")
    TSubclassOf<US_ArenaHUDViewModel> ArenaHUDViewModelClass;

    UPROPERTY(EditDefaultsOnly, Category = "HUDManager|ViewModels")
    TSubclassOf<US_StrafeHUDViewModel> StrafeHUDViewModelClass;

    void CreateViewModels();
    void InitializeViewModels();
    void DeinitializeViewModels();

    // Called when the game state's MatchState changes, to swap GameModeHUDViewModel if necessary
    UFUNCTION() // Make it UFUNCTION if binding to a delegate
        void OnMatchStateChanged(); // Placeholder - needs actual delegate from GameState/GameMode
};