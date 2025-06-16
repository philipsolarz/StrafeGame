// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_PlayerSettingsTab.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_SettingsTabBase.h"
#include "S_UI_PlayerSettingsTab.generated.h"

class UEditableTextBox;
class US_UI_StringComboBox;

/**
 * Player settings tab content.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_PlayerSettingsTab : public US_UI_SettingsTabBase
{
    GENERATED_BODY()

public:
    virtual void ApplySettings() override;
    virtual void RevertSettings() override;
    virtual bool HasUnsavedChanges() const override;

protected:
    virtual void NativeOnInitialized() override;
    virtual void OnViewModelDataChanged() override;

private:
    UFUNCTION()
    void OnPlayerNameChanged(const FText& Text);

    UFUNCTION()
    void OnCharacterModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    // UI Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableTextBox> Txt_PlayerName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<US_UI_StringComboBox> Cmb_CharacterModel;

    // Cached original values for revert functionality
    FString OriginalPlayerName;
    int32 OriginalCharacterModel = 0;

    // Character model options
    TArray<FString> CharacterModelOptions;
};

