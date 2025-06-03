// Source/StrafeGame/Public/UI/MainMenu/Screens/S_CreateGameScreen.h
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Components/ComboBoxString.h" 
#include "UI/MainMenu/MenuScreenInterface.h"
#include "S_CreateGameScreen.generated.h"

class UCommonButtonBase;
class UEditableTextBox;
class UCheckBox;
class USpinBox;
class AS_MainMenuPlayerController; // Changed from UMenuManagerSubsystem

UCLASS()
class STRAFEGAME_API US_CreateGameScreen : public UCommonActivatableWidget, public IMenuScreenInterface
{
    GENERATED_BODY()

public:
    US_CreateGameScreen();
    virtual void NativeConstruct() override;

    // IMenuScreenInterface
    virtual void SetMainMenuPlayerController_Implementation(AS_MainMenuPlayerController* InPlayerController) override; // Updated

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

    void CreateGameSession();
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
    FDelegateHandle CreateSessionCompleteDelegateHandle;

private:
    UPROPERTY()
    TWeakObjectPtr<AS_MainMenuPlayerController> OwningMainMenuPlayerController; // Changed type

    void PopulateGameModes();
    UFUNCTION()
    void OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    void PopulateMapsForGameMode(const FString& GameMode);
};