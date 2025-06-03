// Source/StrafeGame/Public/UI/MainMenu/Screens/S_CreateGameScreen.h
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Components/ComboBoxString.h" // For ESelectInfo::Type
#include "UI/MainMenu/MenuScreenInterface.h"
#include "S_CreateGameScreen.generated.h"

class UCommonButtonBase;
class UEditableTextBox;
//class UComboBoxString; // Already included
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

    // New function with correct signature for delegate binding
    UFUNCTION()
    void OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    // Original logic function
    void PopulateMapsForGameMode(const FString& GameMode);
};