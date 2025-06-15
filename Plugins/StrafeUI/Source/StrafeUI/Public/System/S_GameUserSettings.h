// Plugins/StrafeUI/Source/StrafeUI/Public/System/S_GameUserSettings.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "Data/S_UI_InputTypes.h" // Include the new header for our custom struct
#include "S_GameUserSettings.generated.h"

class FAudioDevice;

/**
 * Custom game user settings class that extends the engine's UGameUserSettings.
 * This class manages all game-specific settings and ensures they persist across sessions.
 */
UCLASS(Config = Game)
class STRAFEUI_API US_GameUserSettings : public UGameUserSettings
{
    GENERATED_BODY()

public:
    US_GameUserSettings();

    /** Get the singleton instance of game user settings */
    UFUNCTION(BlueprintCallable, Category = "Settings")
    static US_GameUserSettings* GetStrafeuiGameUserSettings();

    //~ Begin UGameUserSettings interface
    virtual void SetToDefaults() override;
    virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;
    //~ End UGameUserSettings interface

    //~ Audio Settings
    UPROPERTY(Config, BlueprintReadWrite, Category = "Audio")
    float MasterVolume;

    UPROPERTY(Config, BlueprintReadWrite, Category = "Audio")
    float MusicVolume;

    UPROPERTY(Config, BlueprintReadWrite, Category = "Audio")
    float SFXVolume;

    UPROPERTY(Config, BlueprintReadWrite, Category = "Audio")
    float VoiceVolume;

    //~ Controls Settings
    UPROPERTY(Config, BlueprintReadWrite, Category = "Controls")
    float MouseSensitivity;

    UPROPERTY(Config, BlueprintReadWrite, Category = "Controls")
    bool bInvertYAxis;

    /** Custom key bindings for actions. This will be saved in GameUserSettings.ini */
    UPROPERTY(Config)
    TArray<FStrafeInputActionBinding> CustomKeyBindings;

    //~ Gameplay Settings
    UPROPERTY(Config, BlueprintReadWrite, Category = "Gameplay")
    float FieldOfView;

    UPROPERTY(Config, BlueprintReadWrite, Category = "Gameplay")
    bool bShowFPSCounter;

    //~ Player Settings
    UPROPERTY(Config, BlueprintReadWrite, Category = "Player")
    FString PlayerName;

    UPROPERTY(Config, BlueprintReadWrite, Category = "Player")
    int32 SelectedCharacterModel;

    /** Apply audio settings to the audio system */
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void ApplyAudioSettings();

    /** Apply control settings */
    UFUNCTION(BlueprintCallable, Category = "Controls")
    void ApplyControlSettings();

    /** Apply gameplay settings */
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void ApplyGameplaySettings();

    /** Apply player settings */
    UFUNCTION(BlueprintCallable, Category = "Player")
    void ApplyPlayerSettings();

    /** Load all settings from config */
    virtual void LoadSettings(bool bForceReload = false) override;

    /** Save all settings to config */
    void SaveSettings();

    /** Gets the list of default action mappings. */
    static void GetDefaultActionMappings(TArray<FStrafeInputActionBinding>& OutMappings);

private:
    /** Apply a volume setting to a specific sound class */
    void ApplyVolumeToSoundClass(FAudioDevice* AudioDevice, const FString& SoundClassName, float Volume);
};