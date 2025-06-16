// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_VM_Settings.cpp

#include "ViewModel/S_UI_VM_Settings.h"
#include "System/S_GameUserSettings.h"
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"


void US_UI_VM_Settings::Initialize()
{
    // Load settings from GameUserSettings on initialization
    LoadSettings();
}

void US_UI_VM_Settings::LoadSettings()
{
    US_GameUserSettings* GameSettings = US_GameUserSettings::GetStrafeuiGameUserSettings();
    if (!GameSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get game user settings"));
        return;
    }

    // Load settings from GameUserSettings into our temporary properties
    // Audio
    MasterVolume = GameSettings->MasterVolume;
    MusicVolume = GameSettings->MusicVolume;
    SFXVolume = GameSettings->SFXVolume;
    VoiceVolume = GameSettings->VoiceVolume;

    // Video - Get from parent UGameUserSettings
    bUseVSync = GameSettings->IsVSyncEnabled();
    ShadowQuality = GameSettings->GetShadowQuality();
    TextureQuality = GameSettings->GetTextureQuality();
    AntiAliasingMode = GameSettings->GetAntiAliasingQuality();

    // Get current resolution
    PopulateVideoOptions();
    FIntPoint CurrentRes = GameSettings->GetScreenResolution();
    ResolutionIndex = GetResolutionIndex(CurrentRes);


    // Get window mode
    WindowMode = (int32)GameSettings->GetFullscreenMode();

    // Controls
    MouseSensitivity = GameSettings->MouseSensitivity;
    bInvertYAxis = GameSettings->bInvertYAxis;
    KeyBindings = GameSettings->CustomKeyBindings;

    // Gameplay
    FieldOfView = GameSettings->FieldOfView;
    bShowFPSCounter = GameSettings->bShowFPSCounter;

    // Player
    PlayerName = GameSettings->PlayerName;
    SelectedCharacterModel = GameSettings->SelectedCharacterModel;

    // Cache the current settings
    CacheCurrentSettings();

    // Broadcast to update UI
    BroadcastDataChanged();
}

void US_UI_VM_Settings::ApplySettings()
{
    US_GameUserSettings* GameSettings = US_GameUserSettings::GetStrafeuiGameUserSettings();
    if (!GameSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get game user settings"));
        return;
    }

    // Copy our temporary values to GameUserSettings
    // Audio
    GameSettings->MasterVolume = MasterVolume;
    GameSettings->MusicVolume = MusicVolume;
    GameSettings->SFXVolume = SFXVolume;
    GameSettings->VoiceVolume = VoiceVolume;

    // Video
    GameSettings->SetVSyncEnabled(bUseVSync);
    GameSettings->SetShadowQuality(ShadowQuality);
    GameSettings->SetTextureQuality(TextureQuality);
    GameSettings->SetAntiAliasingQuality(AntiAliasingMode);

    // Set resolution if changed
    if (ResolutionIndex >= 0 && ResolutionIndex < AvailableResolutions.Num())
    {
        GameSettings->SetScreenResolution(AvailableResolutions[ResolutionIndex]);
    }

    // Set window mode
    GameSettings->SetFullscreenMode((EWindowMode::Type)WindowMode);

    // Controls
    GameSettings->MouseSensitivity = MouseSensitivity;
    GameSettings->bInvertYAxis = bInvertYAxis;
    GameSettings->CustomKeyBindings = KeyBindings;

    // Gameplay
    GameSettings->FieldOfView = FieldOfView;
    GameSettings->bShowFPSCounter = bShowFPSCounter;

    // Player
    GameSettings->PlayerName = PlayerName;
    GameSettings->SelectedCharacterModel = SelectedCharacterModel;

    // Apply all settings
    GameSettings->ApplySettings(false);

    // Save to config
    GameSettings->SaveSettings();

    // Update our cached values
    CacheCurrentSettings();

    UE_LOG(LogTemp, Log, TEXT("Settings applied and saved successfully"));
    BroadcastDataChanged();
}

void US_UI_VM_Settings::RevertChanges()
{
    RestoreFromCache();
    BroadcastDataChanged();
    UE_LOG(LogTemp, Log, TEXT("Settings reverted"));
}

void US_UI_VM_Settings::RestoreDefaults()
{
    US_GameUserSettings* GameSettings = US_GameUserSettings::GetStrafeuiGameUserSettings();
    if (!GameSettings)
    {
        return;
    }

    // Reset to defaults
    GameSettings->SetToDefaults();

    // Apply and Save the new defaults
    GameSettings->ApplySettings(false);
    GameSettings->SaveSettings();

    // Reload into our properties to reflect the changes
    LoadSettings();

    UE_LOG(LogTemp, Log, TEXT("Settings restored to defaults"));
}

void US_UI_VM_Settings::PopulateVideoOptions()
{
    if (AvailableResolutions.Num() > 0)
    {
        return;
    }

    // Get available resolutions as FIntPoints
    UKismetSystemLibrary::GetSupportedFullscreenResolutions(AvailableResolutions);

    // Sort resolutions by size (largest first)
    AvailableResolutions.Sort([](const FIntPoint& A, const FIntPoint& B)
        {
            return (A.X * A.Y) > (B.X * B.Y);
        });

    // Populate resolution display strings from the FIntPoint array
    ResolutionOptions.Empty();
    for (const FIntPoint& Resolution : AvailableResolutions)
    {
        ResolutionOptions.Add(FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y));
    }

    // Window mode options
    WindowModeOptions.Empty();
    WindowModeOptions.Add(TEXT("Fullscreen"));
    WindowModeOptions.Add(TEXT("Windowed Fullscreen"));
    WindowModeOptions.Add(TEXT("Windowed"));


    // Quality options
    QualityOptions.Empty();
    QualityOptions.Add(TEXT("Low"));
    QualityOptions.Add(TEXT("Medium"));
    QualityOptions.Add(TEXT("High"));
    QualityOptions.Add(TEXT("Epic"));
    QualityOptions.Add(TEXT("Cinematic"));

    // Anti-aliasing options
    AntiAliasingOptions.Empty();
    AntiAliasingOptions.Add(TEXT("Off"));
    AntiAliasingOptions.Add(TEXT("FXAA"));
    AntiAliasingOptions.Add(TEXT("TAA"));
    AntiAliasingOptions.Add(TEXT("MSAA"));
    AntiAliasingOptions.Add(TEXT("TSR"));
}

int32 US_UI_VM_Settings::GetResolutionIndex(const FIntPoint& Resolution) const
{
    for (int32 i = 0; i < AvailableResolutions.Num(); ++i)
    {
        if (AvailableResolutions[i] == Resolution)
        {
            return i;
        }
    }
    return 0; // Default to first resolution if not found
}

void US_UI_VM_Settings::CacheCurrentSettings()
{
    // Audio
    CachedSettings.MasterVolume = MasterVolume;
    CachedSettings.MusicVolume = MusicVolume;
    CachedSettings.SFXVolume = SFXVolume;
    CachedSettings.VoiceVolume = VoiceVolume;

    // Video
    CachedSettings.bUseVSync = bUseVSync;
    CachedSettings.ShadowQuality = ShadowQuality;
    CachedSettings.TextureQuality = TextureQuality;
    CachedSettings.AntiAliasingMode = AntiAliasingMode;
    CachedSettings.ResolutionIndex = ResolutionIndex;
    CachedSettings.WindowMode = WindowMode;

    // Controls
    CachedSettings.MouseSensitivity = MouseSensitivity;
    CachedSettings.bInvertYAxis = bInvertYAxis;
    CachedSettings.KeyBindings = KeyBindings;

    // Gameplay
    CachedSettings.FieldOfView = FieldOfView;
    CachedSettings.bShowFPSCounter = bShowFPSCounter;

    // Player
    CachedSettings.PlayerName = PlayerName;
    CachedSettings.SelectedCharacterModel = SelectedCharacterModel;
}

void US_UI_VM_Settings::RestoreFromCache()
{
    // Audio
    MasterVolume = CachedSettings.MasterVolume;
    MusicVolume = CachedSettings.MusicVolume;
    SFXVolume = CachedSettings.SFXVolume;
    VoiceVolume = CachedSettings.VoiceVolume;

    // Video
    bUseVSync = CachedSettings.bUseVSync;
    ShadowQuality = CachedSettings.ShadowQuality;
    TextureQuality = CachedSettings.TextureQuality;
    AntiAliasingMode = CachedSettings.AntiAliasingMode;
    ResolutionIndex = CachedSettings.ResolutionIndex;
    WindowMode = CachedSettings.WindowMode;

    // Controls
    MouseSensitivity = CachedSettings.MouseSensitivity;
    bInvertYAxis = CachedSettings.bInvertYAxis;
    KeyBindings = CachedSettings.KeyBindings;

    // Gameplay
    FieldOfView = CachedSettings.FieldOfView;
    bShowFPSCounter = CachedSettings.bShowFPSCounter;

    // Player
    PlayerName = CachedSettings.PlayerName;
    SelectedCharacterModel = CachedSettings.SelectedCharacterModel;
}