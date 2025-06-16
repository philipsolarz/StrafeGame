// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_ControlsSettingsTab.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_SettingsTabBase.h"
#include "Data/S_UI_InputTypes.h"
#include "S_UI_ControlsSettingsTab.generated.h"

class USlider;
class UCheckBox;
class UTextBlock;
class UScrollBox;
class US_UI_KeyBindingWidget;
class UCommonButtonBase;


/**
 * Enhanced controls settings tab with key bindings
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_ControlsSettingsTab : public US_UI_SettingsTabBase
{
    GENERATED_BODY()

public:
    virtual void ApplySettings() override;
    virtual void RevertSettings() override;
    virtual bool HasUnsavedChanges() const override;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void OnViewModelDataChanged() override;

private:
    UFUNCTION()
    void OnMouseSensitivityChanged(float Value);

    UFUNCTION()
    void OnInvertYChanged(bool bIsChecked);

    UFUNCTION()
    void OnKeyBindingChanged(FName ActionName, FKey NewKey);

    UFUNCTION()
    void OnResetToDefaultsClicked();

    void PopulateKeyBindings();
    void CreateKeyBindingWidget(const FStrafeInputActionBinding& Binding);

    // UI Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> Slider_MouseSensitivity;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_MouseSensitivityValue;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCheckBox> Chk_InvertY;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UScrollBox> ScrollBox_KeyBindings;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_ResetToDefaults;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_MovementCategory;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_CombatCategory;

    // Key binding widget class to spawn
    UPROPERTY(EditDefaultsOnly, Category = "Controls")
    TSubclassOf<US_UI_KeyBindingWidget> KeyBindingWidgetClass;

    // Current input bindings, buffered from the ViewModel for UI interaction
    UPROPERTY()
    TArray<FStrafeInputActionBinding> InputBindings;

    // Cached original values to detect changes
    float OriginalMouseSensitivity = 1.0f;
    bool bOriginalInvertY = false;
    TArray<FStrafeInputActionBinding> OriginalInputBindings;

    // References to created key binding widgets
    UPROPERTY()
    TArray<TObjectPtr<US_UI_KeyBindingWidget>> KeyBindingWidgets;
};