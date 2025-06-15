// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_CreateGameWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "Groups/CommonButtonGroupBase.h"
#include "S_UI_CreateGameWidget.generated.h"

class US_UI_VM_CreateGame;
class UEditableTextBox;
class US_UI_StringComboBox;
class UCheckBox;
class USlider;
class US_UI_TextButton;
class US_UI_CollapsibleBox;
class UTextBlock;

UCLASS(Abstract)
class STRAFEUI_API US_UI_CreateGameWidget : public US_UI_BaseScreenWidget
{
    GENERATED_BODY()

public:
    void SetViewModel(US_UI_ViewModelBase* InViewModel);

    virtual US_UI_ViewModelBase* CreateViewModel() override;

protected:
    virtual void NativeOnInitialized() override;

private:
    UFUNCTION()
    void OnViewModelDataChanged();

    UFUNCTION()
    void OnCreateGameClicked();

    UFUNCTION()
    void OnGameModeButtonSelected(UCommonButtonBase* SelectedButton, int32 ButtonIndex);

    UFUNCTION()
    void OnMaxPlayersChanged(float Value);

    UPROPERTY()
    TWeakObjectPtr<US_UI_VM_CreateGame> ViewModel;

    UPROPERTY()
    TObjectPtr<UCommonButtonGroupBase> GameModeButtonGroup; // <<< FIX: Corrected class name

    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* Txt_GameName;

    UPROPERTY(meta = (BindWidget))
    US_UI_TextButton* Btn_ModeStrafe;

    UPROPERTY(meta = (BindWidget))
    US_UI_TextButton* Btn_ModeArena;

    UPROPERTY(meta = (BindWidget))
    US_UI_TextButton* Btn_ModeDuel;

    UPROPERTY(meta = (BindWidget))
    US_UI_StringComboBox* Cmb_Map;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* Chk_IsLAN;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* Chk_IsDedicatedServer;

    UPROPERTY(meta = (BindWidget))
    USlider* Sld_MaxPlayers;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Txt_MaxPlayersValue;

    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* Txt_Password;

    UPROPERTY(meta = (BindWidgetOptional))
    US_UI_CollapsibleBox* Col_GameModeSettings;

    UPROPERTY(meta = (BindWidgetOptional))
    US_UI_CollapsibleBox* Col_AdvancedSettings;

    UPROPERTY(meta = (BindWidget))
    US_UI_TextButton* Btn_CreateGame;
};