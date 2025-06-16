// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_VideoSettingsTab.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_SettingsTabBase.h"
#include "S_UI_VideoSettingsTab.generated.h"

class US_UI_StringComboBox;
class UCheckBox;

/**
 * Video settings tab content.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_VideoSettingsTab : public US_UI_SettingsTabBase
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
    void OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnVSyncChanged(bool bIsChecked);

    UFUNCTION()
    void OnShadowQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnTextureQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnAntiAliasingChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    // UI Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<US_UI_StringComboBox> Cmb_Resolution;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<US_UI_StringComboBox> Cmb_WindowMode;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCheckBox> Chk_VSync;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<US_UI_StringComboBox> Cmb_ShadowQuality;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<US_UI_StringComboBox> Cmb_TextureQuality;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<US_UI_StringComboBox> Cmb_AntiAliasing;

    // Cached original values for revert functionality
    int32 OriginalResolutionIndex = 0;
    int32 OriginalWindowMode = 0;
    bool bOriginalVSync = true;
    int32 OriginalShadowQuality = 2;
    int32 OriginalTextureQuality = 2;
    int32 OriginalAntiAliasingMode = 2;
};