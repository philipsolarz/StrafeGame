// Plugins/StrafeUI/Source/StrafeUI/Private/System/S_GameUserSettings.cpp

#include "System/S_GameUserSettings.h"
#include "Engine/Engine.h"
#include "AudioDevice.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/InputSettings.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

US_GameUserSettings::US_GameUserSettings()
{
    // Set default values in constructor
    SetToDefaults();
}

US_GameUserSettings* US_GameUserSettings::GetStrafeuiGameUserSettings()
{
    return Cast<US_GameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void US_GameUserSettings::GetDefaultActionMappings(TArray<FStrafeInputActionBinding>& OutMappings)
{
    OutMappings.Empty();
    // Movement
    OutMappings.Emplace(FName("MoveForward"), FText::FromString("Move Forward"), EKeys::W, EKeys::Up, TEXT("Movement"));
    OutMappings.Emplace(FName("MoveBackward"), FText::FromString("Move Backward"), EKeys::S, EKeys::Down, TEXT("Movement"));
    OutMappings.Emplace(FName("MoveLeft"), FText::FromString("Move Left"), EKeys::A, EKeys::Left, TEXT("Movement"));
    OutMappings.Emplace(FName("MoveRight"), FText::FromString("Move Right"), EKeys::D, EKeys::Right, TEXT("Movement"));
    OutMappings.Emplace(FName("Jump"), FText::FromString("Jump"), EKeys::SpaceBar, EKeys::Invalid, TEXT("Movement"));
    OutMappings.Emplace(FName("Crouch"), FText::FromString("Crouch"), EKeys::LeftControl, EKeys::C, TEXT("Movement"));
    // Combat
    OutMappings.Emplace(FName("Fire"), FText::FromString("Fire"), EKeys::LeftMouseButton, EKeys::Invalid, TEXT("Combat"));
    OutMappings.Emplace(FName("AltFire"), FText::FromString("Alt Fire / Aim"), EKeys::RightMouseButton, EKeys::Invalid, TEXT("Combat"));
    OutMappings.Emplace(FName("Reload"), FText::FromString("Reload"), EKeys::R, EKeys::Invalid, TEXT("Combat"));
}


void US_GameUserSettings::SetToDefaults()
{
    // Call parent implementation first
    Super::SetToDefaults();

    // Audio defaults
    MasterVolume = 1.0f;
    MusicVolume = 0.7f;
    SFXVolume = 1.0f;
    VoiceVolume = 1.0f;

    // Controls defaults
    MouseSensitivity = 1.0f;
    bInvertYAxis = false;

    // Gameplay defaults
    FieldOfView = 90.0f;
    bShowFPSCounter = false;

    // Player defaults
    PlayerName = TEXT("Player");
    SelectedCharacterModel = 0;

    // Key binding defaults
    GetDefaultActionMappings(CustomKeyBindings);
}

void US_GameUserSettings::ApplySettings(bool bCheckForCommandLineOverrides)
{
    // Apply parent settings (video settings)
    Super::ApplySettings(bCheckForCommandLineOverrides);

    // Apply all our custom settings
    ApplyAudioSettings();
    ApplyControlSettings();
    ApplyGameplaySettings();
    ApplyPlayerSettings();

    // Save to config file
    SaveConfig();
}

void US_GameUserSettings::ApplyAudioSettings()
{
    if (GEngine)
    {
        if (FAudioDeviceHandle AudioDeviceHandle = GEngine->GetMainAudioDevice())
        {
            if (FAudioDevice* AudioDevice = AudioDeviceHandle.GetAudioDevice())
            {
                // Apply volume settings to sound classes
                ApplyVolumeToSoundClass(AudioDevice, TEXT("Master"), MasterVolume);
                ApplyVolumeToSoundClass(AudioDevice, TEXT("Music"), MusicVolume);
                ApplyVolumeToSoundClass(AudioDevice, TEXT("SFX"), SFXVolume);
                ApplyVolumeToSoundClass(AudioDevice, TEXT("Voice"), VoiceVolume);
            }
        }
    }


    UE_LOG(LogTemp, Log, TEXT("Applied audio settings - Master: %.2f, Music: %.2f, SFX: %.2f, Voice: %.2f"),
        MasterVolume, MusicVolume, SFXVolume, VoiceVolume);
}

void US_GameUserSettings::ApplyControlSettings()
{
    // Apply mouse sensitivity and Y-axis inversion to all player controllers
    if (GEngine && GEngine->GetWorld())
    {
        for (FConstPlayerControllerIterator Iterator = GEngine->GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
        {
            if (APlayerController* PC = Iterator->Get())
            {
                if (UPlayerInput* PlayerInput = PC->PlayerInput)
                {
                    PlayerInput->SetMouseSensitivity(MouseSensitivity);
                }
            }
        }
    }

    // --- Apply Key Bindings ---
    UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();
    if (!InputSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get InputSettings for applying key bindings."));
        return;
    }

    // Create a set of unique action names we are managing
    TSet<FName> ManagedActionNames;
    for (const FStrafeInputActionBinding& Binding : CustomKeyBindings)
    {
        ManagedActionNames.Add(Binding.ActionName);
    }

    // Remove all old mappings for the actions we manage
    for (const FName& ActionName : ManagedActionNames)
    {
        TArray<FInputActionKeyMapping> OldMappings;
        InputSettings->GetActionMappingByName(ActionName, OldMappings);
        for (const FInputActionKeyMapping& OldMapping : OldMappings)
        {
            InputSettings->RemoveActionMapping(OldMapping);
        }
    }

    // Add the new mappings from our settings
    for (const FStrafeInputActionBinding& Binding : CustomKeyBindings)
    {
        if (Binding.PrimaryKey.IsValid())
        {
            FInputActionKeyMapping NewMapping(Binding.ActionName, Binding.PrimaryKey);
            InputSettings->AddActionMapping(NewMapping);
        }
        if (Binding.SecondaryKey.IsValid())
        {
            FInputActionKeyMapping NewMapping(Binding.ActionName, Binding.SecondaryKey);
            InputSettings->AddActionMapping(NewMapping);
        }
    }

    // Save the changes to the config file (Input.ini)
    InputSettings->SaveKeyMappings();

    // Rebuild keymaps for all active players to apply changes immediately
    if (GEngine && GEngine->GetWorld())
    {
        for (FConstPlayerControllerIterator It = GEngine->GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            if (APlayerController* PC = It->Get())
            {
                if (PC->PlayerInput)
                {
                    PC->PlayerInput->ForceRebuildingKeyMaps(true);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Applied control settings - Mouse Sensitivity: %.2f, Invert Y: %s"),
        MouseSensitivity, bInvertYAxis ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Applied and rebuilt custom key bindings."));
}

void US_GameUserSettings::ApplyGameplaySettings()
{
    // Apply FOV to all player controllers
    if (GEngine && GEngine->GetWorld())
    {
        for (FConstPlayerControllerIterator Iterator = GEngine->GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
        {
            if (APlayerController* PC = Iterator->Get())
            {
                PC->FOV(FieldOfView);
            }
        }
    }

    // Apply FPS counter setting
    if (GEngine)
    {
        GEngine->bEnableOnScreenDebugMessages = bShowFPSCounter;
        if (bShowFPSCounter)
        {
            GEngine->Exec(nullptr, TEXT("stat fps"));
        }
        else
        {
            GEngine->Exec(nullptr, TEXT("stat none"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Applied gameplay settings - FOV: %.1f, Show FPS: %s"),
        FieldOfView, bShowFPSCounter ? TEXT("Yes") : TEXT("No"));
}

void US_GameUserSettings::ApplyPlayerSettings()
{
    // In a real game, you would apply player name and character model here
    // For now, just log the settings
    UE_LOG(LogTemp, Log, TEXT("Applied player settings - Name: %s, Character Model: %d"),
        *PlayerName, SelectedCharacterModel);
}

void US_GameUserSettings::LoadSettings(bool bForceReload)
{
    // Load parent settings
    Super::LoadSettings(bForceReload);

    // If custom keybindings are not loaded from config (e.g., first run), populate with defaults.
    if (CustomKeyBindings.Num() == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("No custom key bindings found in config, loading defaults."));
        GetDefaultActionMappings(CustomKeyBindings);
    }

    // Our custom settings are automatically loaded from config due to UPROPERTY(Config)
    // But we should validate them
    MasterVolume = FMath::Clamp(MasterVolume, 0.0f, 1.0f);
    MusicVolume = FMath::Clamp(MusicVolume, 0.0f, 1.0f);
    SFXVolume = FMath::Clamp(SFXVolume, 0.0f, 1.0f);
    VoiceVolume = FMath::Clamp(VoiceVolume, 0.0f, 1.0f);

    MouseSensitivity = FMath::Clamp(MouseSensitivity, 0.1f, 3.0f);

    FieldOfView = FMath::Clamp(FieldOfView, 60.0f, 120.0f);

    SelectedCharacterModel = FMath::Clamp(SelectedCharacterModel, 0, 10);

    UE_LOG(LogTemp, Log, TEXT("Loaded user settings from config"));
}

void US_GameUserSettings::SaveSettings()
{
    SaveConfig();
    UE_LOG(LogTemp, Log, TEXT("Saved user settings to config"));
}

void US_GameUserSettings::ApplyVolumeToSoundClass(FAudioDevice* AudioDevice, const FString& SoundClassName, float Volume)
{
    if (!AudioDevice)
    {
        return;
    }

    // In a real implementation, you would have references to your sound classes
    // For now, we'll use a simple approach
    FString SoundClassPath = FString::Printf(TEXT("/Game/Audio/SoundClasses/SC_%s.SC_%s"), *SoundClassName, *SoundClassName);
    USoundClass* SoundClass = LoadObject<USoundClass>(nullptr, *SoundClassPath);

    if (SoundClass)
    {
        AudioDevice->SetSoundMixClassOverride(GetMutableDefault<USoundMix>(), SoundClass, Volume, 1.0f, 0.0f, true);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not find sound class: %s"), *SoundClassName);
    }
}