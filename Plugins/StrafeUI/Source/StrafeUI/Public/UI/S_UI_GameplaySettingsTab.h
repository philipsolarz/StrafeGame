// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_GameplaySettingsTab.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_SettingsTabBase.h"
#include "S_UI_GameplaySettingsTab.generated.h"

class USlider;
class UCheckBox;
class UTextBlock;

/**
 * Gameplay settings tab content.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_GameplaySettingsTab : public US_UI_SettingsTabBase
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
    void OnFieldOfViewChanged(float Value);

    UFUNCTION()
    void OnShowFPSChanged(bool bIsChecked);

    // UI Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> Slider_FieldOfView;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_FieldOfViewValue;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCheckBox> Chk_ShowFPS;

    // Cached original values for revert functionality
    float OriginalFieldOfView = 90.0f;
    bool bOriginalShowFPS = false;
};

