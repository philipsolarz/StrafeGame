// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CommonActivatableWidget.h"
#include "InputMappingContext.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "MenuManagerSubsystem.generated.h"

class US_MainMenuScreen;
class US_ConfirmDialogWidget;
class UInputAction;

// This delegate is now primarily for Blueprint binding and C++ internal broadcast
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConfirmDialogResultSet, bool, bConfirmed);

UCLASS()
class STRAFEGAME_API UMenuManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Menu Manager")
    void ToggleMainMenu();

    UFUNCTION(BlueprintCallable, Category = "Menu Manager")
    void ShowScreen(TSubclassOf<UCommonActivatableWidget> ScreenWidgetClass);

    UFUNCTION(BlueprintCallable, Category = "Menu Manager")
    void CloseTopmostScreen();

    // Changed: Removed FOnConfirmDialogClosed parameter for BP compatibility
    UFUNCTION(BlueprintCallable, Category = "Menu Manager")
    void ShowConfirmDialog(const FText& Title, const FText& Message);

    // This delegate is broadcast when a confirmation dialog shown via this subsystem is closed.
    // C++ callers should bind to this BEFORE calling ShowConfirmDialog if they need a callback.
    UPROPERTY(BlueprintAssignable, Category = "Menu Manager|Events")
    FOnConfirmDialogResultSet OnConfirmDialogResultSet; // Renamed

    UFUNCTION(BlueprintPure, Category = "Menu Manager|Input")
    UInputAction* GetToggleMenuAction() const { return ToggleMenuAction; }

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Menu Manager|Config")
    TSoftClassPtr<US_MainMenuScreen> MainMenuScreenClass;

    UPROPERTY(EditDefaultsOnly, Category = "Menu Manager|Config")
    TSoftClassPtr<US_ConfirmDialogWidget> ConfirmDialogClass;

    UPROPERTY(EditDefaultsOnly, Category = "Menu Manager|Input")
    TSoftObjectPtr<UInputMappingContext> MenuInputMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Menu Manager|Input")
    TObjectPtr<UInputAction> ToggleMenuAction;

private:
    UPROPERTY()
    TObjectPtr<UCommonActivatableWidgetStack> ActiveWidgetStack;

    UPROPERTY()
    TObjectPtr<US_MainMenuScreen> MainMenuScreenInstance;

    // This internal handler is called by US_ConfirmDialogWidget
    void HandleConfirmDialogClosedInternal(bool bConfirmed);

    bool bIsMenuOpen;
};