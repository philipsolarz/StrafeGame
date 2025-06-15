// Plugins/StrafeUI/Source/StrafeUI/Public/ViewModel/S_UI_VM_Settings.h

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/S_UI_ViewModelBase.h"
#include "Data/S_UI_InputTypes.h"
#include "S_UI_VM_Settings.generated.h"

/**
 * @class US_UI_VM_Settings
 * @brief ViewModel for the Settings screen.
 *
 * Acts as a temporary buffer between the UI and the persistent US_GameUserSettings.
 * Handles the data and logic for all game settings categories.
 */
UCLASS(BlueprintType)
class STRAFEUI_API US_UI_VM_Settings : public US_UI_ViewModelBase
{
    GENERATED_BODY()

public:
    /** Initialize the view model and load settings */
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void Initialize();

    //~ Audio Settings (temporary buffer)
    /** The master volume level (0.0 to 1.0). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Audio")
    float MasterVolume = 1.0f;

    /** Music volume level (0.0 to 1.0). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Audio")
    float MusicVolume = 1.0f;

    /** Sound effects volume level (0.0 to 1.0). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Audio")
    float SFXVolume = 1.0f;

    /** Voice/dialogue volume level (0.0 to 1.0). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Audio")
    float VoiceVolume = 1.0f;

    //~ Video Settings (temporary buffer)
    /** Whether VSync is enabled. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
    bool bUseVSync = true;

    /** Shadow quality level (0=Low, 1=Medium, 2=High, 3=Ultra). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
    int32 ShadowQuality = 2;

    /** Texture quality level (0=Low, 1=Medium, 2=High, 3=Ultra). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
    int32 TextureQuality = 2;

    /** Anti-aliasing mode (0=Off, 1=FXAA, 2=TAA, 3=MSAA). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
    int32 AntiAliasingMode = 2;

    /** Display resolution index. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
    int32 ResolutionIndex = 0;

    /** Window mode (0=Fullscreen, 1=Windowed, 2=Borderless). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
    int32 WindowMode = 0;

    //~ Controls Settings (temporary buffer)
    /** Mouse sensitivity (0.1 to 3.0). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Controls")
    float MouseSensitivity = 1.0f;

    /** Whether to invert Y axis. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Controls")
    bool bInvertYAxis = false;

    /** Key bindings for game actions. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Controls")
    TArray<FStrafeInputActionBinding> KeyBindings;

    //~ Gameplay Settings (temporary buffer)
    /** Field of view (60 to 120). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Gameplay")
    float FieldOfView = 90.0f;

    /** Whether to show FPS counter. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Gameplay")
    bool bShowFPSCounter = false;

    //~ Player Settings (temporary buffer)
    /** Player display name. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Player")
    FString PlayerName = TEXT("Player");

    /** Selected character model index. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Player")
    int32 SelectedCharacterModel = 0;

    //~ Video Options Lists
    /** Available screen resolutions. */
    UPROPERTY(BlueprintReadOnly, Category = "Settings|Video")
    TArray<FIntPoint> AvailableResolutions;

    /** Resolution options as display strings. */
    UPROPERTY(BlueprintReadOnly, Category = "Settings|Video")
    TArray<FString> ResolutionOptions;

    /** Window mode options. */
    UPROPERTY(BlueprintReadOnly, Category = "Settings|Video")
    TArray<FString> WindowModeOptions;

    /** Quality level options. */
    UPROPERTY(BlueprintReadOnly, Category = "Settings|Video")
    TArray<FString> QualityOptions;

    /** Anti-aliasing options. */
    UPROPERTY(BlueprintReadOnly, Category = "Settings|Video")
    TArray<FString> AntiAliasingOptions;

    /**
     * Loads settings from US_GameUserSettings into the temporary buffer.
     */
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void LoadSettings();

    /**
     * Applies the current settings from the temporary buffer to US_GameUserSettings.
     */
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ApplySettings();

    /**
     * Reverts any pending changes back to their last applied state.
     */
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void RevertChanges();

    /**
     * Restores all settings to their default values.
     */
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void RestoreDefaults();

    /**
     * Populates video option lists (resolutions, etc.).
     */
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void PopulateVideoOptions();

private:
    // Cached settings for reverting
    struct FCachedSettings
    {
        // Audio
        float MasterVolume = 1.0f;
        float MusicVolume = 1.0f;
        float SFXVolume = 1.0f;
        float VoiceVolume = 1.0f;

        // Video
        bool bUseVSync = true;
        int32 ShadowQuality = 2;
        int32 TextureQuality = 2;
        int32 AntiAliasingMode = 2;
        int32 ResolutionIndex = 0;
        int32 WindowMode = 0;

        // Controls
        float MouseSensitivity = 1.0f;
        bool bInvertYAxis = false;
        TArray<FStrafeInputActionBinding> KeyBindings;

        // Gameplay
        float FieldOfView = 90.0f;
        bool bShowFPSCounter = false;

        // Player
        FString PlayerName = TEXT("Player");
        int32 SelectedCharacterModel = 0;
    };

    FCachedSettings CachedSettings;

    void CacheCurrentSettings();
    void RestoreFromCache();
    int32 GetResolutionIndex(const FIntPoint& Resolution) const;
};