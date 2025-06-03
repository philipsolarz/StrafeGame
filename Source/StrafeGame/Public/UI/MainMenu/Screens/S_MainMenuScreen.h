// Source/StrafeGame/Public/UI/MainMenu/Screens/S_MainMenuScreen.h
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "UI/MainMenu/MenuScreenInterface.h"
#include "S_MainMenuScreen.generated.h"

class UCommonButtonBase;
class UMenuManagerSubsystem;
class UCommonActivatableWidgetStack; // Added forward declaration
class UCommonActivatableWidget;    // Added forward declaration

UCLASS()
class STRAFEGAME_API US_MainMenuScreen : public UCommonActivatableWidget, public IMenuScreenInterface
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // IMenuScreenInterface
    virtual void SetMenuManager_Implementation(UMenuManagerSubsystem* InMenuManager) override;

    // Function to push a new screen onto this main menu's stack
    UFUNCTION(BlueprintCallable, Category = "MainMenuScreen")
    void PushScreenToStack(TSubclassOf<UCommonActivatableWidget> ScreenWidgetClass);

    // Function to pop the topmost screen from this main menu's stack
    UFUNCTION(BlueprintCallable, Category = "MainMenuScreen")
    void PopScreenFromStack();

    // Getter for the primary widget stack
    UFUNCTION(BlueprintPure, Category = "MainMenuScreen")
    UCommonActivatableWidgetStack* GetPrimaryWidgetStack() const { return PrimaryWidgetStack; }

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_CreateGame;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_FindGame;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Replays;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Settings;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_QuitGame;

    // This UPROPERTY must match the name of the UCommonActivatableWidgetStack in your WBP_MainMenuScreen
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonActivatableWidgetStack> PrimaryWidgetStack;

    UFUNCTION()
    void OnCreateGameClicked();

    UFUNCTION()
    void OnFindGameClicked();

    UFUNCTION()
    void OnReplaysClicked();

    UFUNCTION()
    void OnSettingsClicked();

    UFUNCTION()
    void OnQuitGameClicked();

    UFUNCTION()
    void OnQuitGameConfirmed(bool bConfirmed);

private:
    UPROPERTY()
    TObjectPtr<UMenuManagerSubsystem> MenuManager;
};