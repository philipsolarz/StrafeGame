// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "UI/MainMenu/MenuScreenInterface.h" // Updated include
#include "S_CreateGameScreen.generated.h"

class UCommonButtonBase;
class UEditableTextBox;
class UComboBoxString;
class UCheckBox;
class USpinBox;
class UMenuManagerSubsystem;

UCLASS()
class STRAFEGAME_API US_CreateGameScreen : public UCommonActivatableWidget, public IMenuScreenInterface
{
    GENERATED_BODY()

public:
    US_CreateGameScreen();
    virtual void NativeConstruct() override;

    // IMenuScreenInterface
    virtual void SetMenuManager_Implementation(UMenuManagerSubsystem* InMenuManager) override;

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UComboBoxString> CBox_GameMode;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UComboBoxString> CBox_Map;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCheckBox> Chk_IsPublic;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UEditableTextBox> Txt_GameName;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UEditableTextBox> Txt_Password;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<USpinBox> Spin_MaxPlayers;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_StartGame;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Back;

    UFUNCTION()
    void OnStartGameClicked();

    UFUNCTION()
    void OnBackClicked();

    bool ValidateSettings() const;

    // Online Session
    void CreateGameSession();
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
    FDelegateHandle CreateSessionCompleteDelegateHandle;

private:
    UPROPERTY()
    TObjectPtr<UMenuManagerSubsystem> MenuManager;

    void PopulateGameModes();

    UFUNCTION() // Must be UFUNCTION for delegate binding
        void PopulateMapsForGameMode(const FString& GameMode);
};