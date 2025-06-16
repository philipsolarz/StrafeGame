// Plugins/StrafeUI/Source/StrafeUI/Public/System/S_SettingsPresets.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "S_SettingsPresets.generated.h"

/**
 * Represents a single settings preset
 */
USTRUCT(BlueprintType)
struct FSettingsPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Description;

    // Video Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Video")
    int32 ShadowQuality = 2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Video")
    int32 TextureQuality = 2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Video")
    int32 AntiAliasingMode = 2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Video")
    bool bUseVSync = true;

    // Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    float SFXVolume = 1.0f;

    FSettingsPreset()
    {
        PresetName = "Default";
        DisplayName = FText::FromString("Default");
        Description = FText::FromString("Default settings configuration");
    }
};

/**
 * Data asset containing predefined settings presets
 */
UCLASS()
class STRAFEUI_API US_SettingsPresets : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presets")
    TArray<FSettingsPreset> Presets;

    /**
     * Get a preset by name
     */
    UFUNCTION(BlueprintCallable, Category = "Settings Presets")
    bool GetPresetByName(const FString& PresetName, FSettingsPreset& OutPreset) const;

    /**
     * Apply a preset to the game user settings
     */
    UFUNCTION(BlueprintCallable, Category = "Settings Presets")
    static void ApplyPreset(const FSettingsPreset& Preset);
};

