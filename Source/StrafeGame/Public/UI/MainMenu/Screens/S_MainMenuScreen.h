// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "UI/MainMenu/MenuScreenInterface.h" // Updated include
#include "S_MainMenuScreen.generated.h"

class UCommonButtonBase;
class UMenuManagerSubsystem;

UCLASS()
class STRAFEGAME_API US_MainMenuScreen : public UCommonActivatableWidget, public IMenuScreenInterface
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // IMenuScreenInterface
    virtual void SetMenuManager_Implementation(UMenuManagerSubsystem* InMenuManager) override;

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

    void OnQuitGameConfirmed(bool bConfirmed);

private:
    UPROPERTY()
    TObjectPtr<UMenuManagerSubsystem> MenuManager;
};