// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_AudioSettingsTab.cpp

#include "UI/S_UI_AudioSettingsTab.h"
#include "Components/Slider.h"
#include "ViewModel/S_UI_VM_Settings.h"

void US_UI_AudioSettingsTab::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Bind slider events
    if (Slider_MasterVolume)
    {
        Slider_MasterVolume->OnValueChanged.AddDynamic(this, &US_UI_AudioSettingsTab::OnMasterVolumeChanged);
    }
    if (Slider_MusicVolume)
    {
        Slider_MusicVolume->OnValueChanged.AddDynamic(this, &US_UI_AudioSettingsTab::OnMusicVolumeChanged);
    }
    if (Slider_SFXVolume)
    {
        Slider_SFXVolume->OnValueChanged.AddDynamic(this, &US_UI_AudioSettingsTab::OnSFXVolumeChanged);
    }
    if (Slider_VoiceVolume)
    {
        Slider_VoiceVolume->OnValueChanged.AddDynamic(this, &US_UI_AudioSettingsTab::OnVoiceVolumeChanged);
    }
}

void US_UI_AudioSettingsTab::OnViewModelDataChanged()
{
    if (ViewModel.IsValid())
    {
        // Update UI from view model
        if (Slider_MasterVolume)
        {
            Slider_MasterVolume->SetValue(ViewModel->MasterVolume);
            OriginalMasterVolume = ViewModel->MasterVolume;
        }
        if (Slider_MusicVolume)
        {
            Slider_MusicVolume->SetValue(ViewModel->MusicVolume);
            OriginalMusicVolume = ViewModel->MusicVolume;
        }
        if (Slider_SFXVolume)
        {
            Slider_SFXVolume->SetValue(ViewModel->SFXVolume);
            OriginalSFXVolume = ViewModel->SFXVolume;
        }
        if (Slider_VoiceVolume)
        {
            Slider_VoiceVolume->SetValue(ViewModel->VoiceVolume);
            OriginalVoiceVolume = ViewModel->VoiceVolume;
        }
    }
}

void US_UI_AudioSettingsTab::OnMasterVolumeChanged(float Value)
{
    if (ViewModel.IsValid())
    {
        ViewModel->MasterVolume = Value;
    }
}

void US_UI_AudioSettingsTab::OnMusicVolumeChanged(float Value)
{
    if (ViewModel.IsValid())
    {
        ViewModel->MusicVolume = Value;
    }
}

void US_UI_AudioSettingsTab::OnSFXVolumeChanged(float Value)
{
    if (ViewModel.IsValid())
    {
        ViewModel->SFXVolume = Value;
    }
}

void US_UI_AudioSettingsTab::OnVoiceVolumeChanged(float Value)
{
    if (ViewModel.IsValid())
    {
        ViewModel->VoiceVolume = Value;
    }
}

void US_UI_AudioSettingsTab::ApplySettings()
{
    // Update original values to current values
    if (ViewModel.IsValid())
    {
        OriginalMasterVolume = ViewModel->MasterVolume;
        OriginalMusicVolume = ViewModel->MusicVolume;
        OriginalSFXVolume = ViewModel->SFXVolume;
        OriginalVoiceVolume = ViewModel->VoiceVolume;
    }
}

void US_UI_AudioSettingsTab::RevertSettings()
{
    if (ViewModel.IsValid())
    {
        ViewModel->MasterVolume = OriginalMasterVolume;
        ViewModel->MusicVolume = OriginalMusicVolume;
        ViewModel->SFXVolume = OriginalSFXVolume;
        ViewModel->VoiceVolume = OriginalVoiceVolume;

        // Update UI
        OnViewModelDataChanged();
    }
}

bool US_UI_AudioSettingsTab::HasUnsavedChanges() const
{
    if (!ViewModel.IsValid())
    {
        return false;
    }

    return FMath::Abs(ViewModel->MasterVolume - OriginalMasterVolume) > KINDA_SMALL_NUMBER ||
        FMath::Abs(ViewModel->MusicVolume - OriginalMusicVolume) > KINDA_SMALL_NUMBER ||
        FMath::Abs(ViewModel->SFXVolume - OriginalSFXVolume) > KINDA_SMALL_NUMBER ||
        FMath::Abs(ViewModel->VoiceVolume - OriginalVoiceVolume) > KINDA_SMALL_NUMBER;
}