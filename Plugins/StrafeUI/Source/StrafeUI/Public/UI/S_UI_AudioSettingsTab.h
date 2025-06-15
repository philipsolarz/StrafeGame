// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_AudioSettingsTab.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_SettingsTabBase.h"
#include "S_UI_AudioSettingsTab.generated.h"

class USlider;
class UCheckBox;

/**
 * Audio settings tab content.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_AudioSettingsTab : public US_UI_SettingsTabBase
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
    void OnMasterVolumeChanged(float Value);

    UFUNCTION()
    void OnMusicVolumeChanged(float Value);

    UFUNCTION()
    void OnSFXVolumeChanged(float Value);

    UFUNCTION()
    void OnVoiceVolumeChanged(float Value);

    // UI Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> Slider_MasterVolume;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> Slider_MusicVolume;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> Slider_SFXVolume;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> Slider_VoiceVolume;

    // Cached original values for revert functionality
    float OriginalMasterVolume = 1.0f;
    float OriginalMusicVolume = 1.0f;
    float OriginalSFXVolume = 1.0f;
    float OriginalVoiceVolume = 1.0f;
};