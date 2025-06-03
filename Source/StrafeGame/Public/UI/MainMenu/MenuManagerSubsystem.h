// Source/StrafeGame/Public/UI/MainMenu/MenuManagerSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CommonActivatableWidget.h"    // For TSubclassOf
#include "InputMappingContext.h"        // For UInputMappingContext
#include "MenuManagerSubsystem.generated.h"

class US_MainMenuScreen;
class US_ConfirmDialogWidget;
class UInputAction;
class APlayerController; // Added forward declaration

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConfirmDialogResultSet, bool, bConfirmed);

UCLASS()
class STRAFEGAME_API UMenuManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Changed: Takes PlayerController to correctly associate the menu.
    UFUNCTION(BlueprintCallable, Category = "Menu Manager")
    void ShowInitialMenu(APlayerController* ForPlayerController);

    UFUNCTION(BlueprintCallable, Category = "Menu Manager")
    void CloseMainMenu(APlayerController* ForPlayerController); // Optional: if you need to explicitly close it

    UFUNCTION(BlueprintCallable, Category = "Menu Manager")
    void ShowScreen(TSubclassOf<UCommonActivatableWidget> ScreenWidgetClass);

    UFUNCTION(BlueprintCallable, Category = "Menu Manager")
    void CloseTopmostScreen();

    UFUNCTION(BlueprintCallable, Category = "Menu Manager")
    void ShowConfirmDialog(const FText& Title, const FText& Message);

    UPROPERTY(BlueprintAssignable, Category = "Menu Manager|Events")
    FOnConfirmDialogResultSet OnConfirmDialogResultSet;

    // Getter for ToggleMenuAction might not be relevant if menu is always shown in its own level.
    // UFUNCTION(BlueprintPure, Category = "Menu Manager|Input")
    // UInputAction* GetToggleMenuAction() const { return ToggleMenuAction; }

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Menu Manager|Config")
    TSoftClassPtr<US_MainMenuScreen> MainMenuScreenClass;

    UPROPERTY(EditDefaultsOnly, Category = "Menu Manager|Config")
    TSoftClassPtr<US_ConfirmDialogWidget> ConfirmDialogClass;

    UPROPERTY(EditDefaultsOnly, Category = "Menu Manager|Input", meta = (Tooltip = "Input Mapping Context to apply when menu is active."))
    TSoftObjectPtr<UInputMappingContext> MenuInputMappingContext;

    // ToggleMenuAction might be repurposed or removed if menu is always on in its dedicated level.
    // UPROPERTY(EditDefaultsOnly, Category = "Menu Manager|Input")
    // TObjectPtr<UInputAction> ToggleMenuAction;

private:
    UPROPERTY() // This instance is per-local player if supporting split-screen, otherwise, one instance.
        TObjectPtr<US_MainMenuScreen> MainMenuScreenInstance;

    // For the confirmation dialog
    UPROPERTY()
    TObjectPtr<US_ConfirmDialogWidget> ActiveConfirmDialog;


    UFUNCTION()
    void HandleConfirmDialogClosedInternal(bool bConfirmed);

    // Helper to get the player controller, assumes single local player for now.
    APlayerController* GetLocalPlayerController() const;
};