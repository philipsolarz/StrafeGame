// Plugins/StrafeUI/Source/StrafeUI/Private/System/S_SettingsPresets.cpp

#include "System/S_SettingsPresets.h"
#include "System/S_GameUserSettings.h"

bool US_SettingsPresets::GetPresetByName(const FString& PresetName, FSettingsPreset& OutPreset) const
{
    for (const FSettingsPreset& Preset : Presets)
    {
        if (Preset.PresetName == PresetName)
        {
            OutPreset = Preset;
            return true;
        }
    }
    return false;
}

void US_SettingsPresets::ApplyPreset(const FSettingsPreset& Preset)
{
    US_GameUserSettings* GameSettings = US_GameUserSettings::GetStrafeuiGameUserSettings();
    if (!GameSettings)
    {
        return;
    }

    // Apply video settings
    GameSettings->SetShadowQuality(Preset.ShadowQuality);
    GameSettings->SetTextureQuality(Preset.TextureQuality);
    GameSettings->SetAntiAliasingQuality(Preset.AntiAliasingMode);
    GameSettings->SetVSyncEnabled(Preset.bUseVSync);

    // Apply audio settings
    GameSettings->MasterVolume = Preset.MasterVolume;
    GameSettings->MusicVolume = Preset.MusicVolume;
    GameSettings->SFXVolume = Preset.SFXVolume;

    // Apply all settings
    GameSettings->ApplySettings(false);
    GameSettings->SaveSettings();

    UE_LOG(LogTemp, Log, TEXT("Applied settings preset: %s"), *Preset.PresetName);
}