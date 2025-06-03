// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h" // For UInputMappingContext
#include "S_MainMenuPlayerController.generated.h"

class US_MainMenuScreen;
class US_ConfirmDialogWidget;
class UCommonActivatableWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConfirmDialogResultSetNative, bool, bConfirmed);

/**
 * PlayerController for the Main Menu level.
 * Manages the main menu UI lifecycle and global menu actions.
 */
UCLASS()
class STRAFEGAME_API AS_MainMenuPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AS_MainMenuPlayerController();

    UFUNCTION(BlueprintCallable, Category = "MainMenu|Navigation")
    void ShowScreen(TSubclassOf<UCommonActivatableWidget> ScreenWidgetClass);

    UFUNCTION(BlueprintCallable, Category = "MainMenu|Navigation")
    void CloseTopmostScreen();

    UFUNCTION(BlueprintCallable, Category = "MainMenu|Dialogs")
    void ShowConfirmDialog(const FText& Title, const FText& Message);

    UPROPERTY(BlueprintAssignable, Category = "MainMenu|Events")
    FOnConfirmDialogResultSetNative OnConfirmDialogResultSet;

    UFUNCTION(BlueprintCallable, Category = "MainMenu")
    void CloseFullMenu();


protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditDefaultsOnly, Category = "MainMenu|Config")
    TSoftClassPtr<UCommonActivatableWidget> MainMenuScreenClass;

    UPROPERTY(EditDefaultsOnly, Category = "MainMenu|Config")
    TSoftClassPtr<US_ConfirmDialogWidget> ConfirmDialogClass;

    UPROPERTY(EditDefaultsOnly, Category = "MainMenu|Input", meta = (Tooltip = "Input Mapping Context to apply when menu is active."))
    TSoftObjectPtr<UInputMappingContext> MenuInputMappingContext;

private:
    UPROPERTY()
    TObjectPtr<US_MainMenuScreen> MainMenuScreenInstance;

    UPROPERTY()
    TObjectPtr<US_ConfirmDialogWidget> ActiveConfirmDialog;

    void ShowInitialMenu();

    UFUNCTION()
    void HandleConfirmDialogClosedInternal(bool bConfirmed);
};