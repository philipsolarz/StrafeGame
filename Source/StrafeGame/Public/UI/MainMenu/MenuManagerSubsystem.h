// Source/StrafeGame/Public/UI/MainMenu/MenuManagerSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CommonActivatableWidget.h"
#include "InputMappingContext.h"
#include "Widgets/CommonActivatableWidgetContainer.h" // Corrected path if it was different
#include "MenuManagerSubsystem.generated.h"

class US_MainMenuScreen;
class US_ConfirmDialogWidget;
class UInputAction;
class UCommonActivatableWidgetStack; // Forward declare


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

    UFUNCTION(BlueprintCallable, Category = "Menu Manager")
    void ShowConfirmDialog(const FText& Title, const FText& Message);

    UPROPERTY(BlueprintAssignable, Category = "Menu Manager|Events")
    FOnConfirmDialogResultSet OnConfirmDialogResultSet;

    UFUNCTION(BlueprintPure, Category = "Menu Manager|Input")
    UInputAction* GetToggleMenuAction() const { return ToggleMenuAction; }

    UFUNCTION(BlueprintPure, Category = "Menu Manager|Internal")
    UCommonActivatableWidgetStack* GetActiveWidgetStack() const;


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

    UFUNCTION() // Ensure this is a UFUNCTION for AddDynamic
        void HandleConfirmDialogClosedInternal(bool bConfirmed);

    bool bIsMenuOpen;
};