// Source/StrafeGame/Public/UI/MainMenu/Screens/S_MainMenuScreen.h
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "UI/MainMenu/MenuScreenInterface.h"
#include "S_MainMenuScreen.generated.h"

class UCommonButtonBase;
class AS_MainMenuPlayerController; // Changed from UMenuManagerSubsystem
class UCommonActivatableWidgetStack;
class UCommonActivatableWidget;

UCLASS()
class STRAFEGAME_API US_MainMenuScreen : public UCommonActivatableWidget, public IMenuScreenInterface
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // IMenuScreenInterface
    virtual void SetMainMenuPlayerController_Implementation(AS_MainMenuPlayerController* InPlayerController) override; // Updated

    UFUNCTION(BlueprintCallable, Category = "MainMenuScreen")
    void PushScreenToStack(TSubclassOf<UCommonActivatableWidget> ScreenWidgetClass);

    UFUNCTION(BlueprintCallable, Category = "MainMenuScreen")
    void PopScreenFromStack();

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
    TWeakObjectPtr<AS_MainMenuPlayerController> OwningMainMenuPlayerController; // Changed type
};