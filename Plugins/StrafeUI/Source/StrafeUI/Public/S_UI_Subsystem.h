#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/S_UI_ScreenTypes.h"
#include "S_UI_Subsystem.generated.h"

class US_UI_AssetManager;
class US_UI_Navigator;
class US_UI_InputController;
class US_UI_ModalStack;
class US_UI_RootWidget;
class AS_UI_PlayerController;
class US_UI_OnlineSessionManager;

/**
 * The central orchestrator for the StrafeUI plugin.
 * Initializes and provides access to specialized UI managers.
 */
UCLASS()
class STRAFEUI_API US_UI_Subsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Kicks off the full UI initialization for a given player. */
    void InitializeUIForPlayer(AS_UI_PlayerController* PlayerController);

    /** Requests a modal dialog to be displayed. */
    void RequestModal(const F_UIModalPayload& Payload, const FOnModalDismissedSignature& OnDismissedCallback);

    /** Gets the root UI widget. */
    UFUNCTION(BlueprintPure, Category = "UI Subsystem")
    US_UI_RootWidget* GetRootWidget() const { return UIRootWidget; }

    /** Gets the screen navigation manager. */
    UFUNCTION(BlueprintPure, Category = "UI Subsystem")
    US_UI_Navigator* GetNavigator() const { return Navigator; }

    /** Gets the UI asset loading manager. */
    UFUNCTION(BlueprintPure, Category = "UI Subsystem")
    US_UI_AssetManager* GetAssetManager() const { return AssetManager; }

    /** Gets the online session manager. */
    UFUNCTION(BlueprintPure, Category = "UI Subsystem")
    US_UI_OnlineSessionManager* GetSessionManager() const { return SessionManager; }

private:
    /** Finalizes UI setup after all assets have been loaded. */
    void FinalizeUIInitialization();

    /** Manager for loading UI assets. */
    UPROPERTY()
    TObjectPtr<US_UI_AssetManager> AssetManager;

    /** Manager for screen navigation. */
    UPROPERTY()
    TObjectPtr<US_UI_Navigator> Navigator;

    /** Manager for UI input. */
    UPROPERTY()
    TObjectPtr<US_UI_InputController> InputController;

    /** Manager for the modal dialog queue. */
    UPROPERTY()
    TObjectPtr<US_UI_ModalStack> ModalStack;

    /** Manager for online sessions. */
    UPROPERTY()
    TObjectPtr<US_UI_OnlineSessionManager> SessionManager;

    /** The root widget of the UI. */
    UPROPERTY()
    TObjectPtr<US_UI_RootWidget> UIRootWidget;

    /** A weak pointer to the player controller that is initializing the UI. */
    UPROPERTY()
    TWeakObjectPtr<AS_UI_PlayerController> InitializingPlayer;
};