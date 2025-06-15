#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/StreamableManager.h"
#include "Data/S_UI_ScreenTypes.h"
#include "S_UI_AssetManager.generated.h"

class US_UI_Settings;
class UCommonActivatableWidget;

// Delegate to broadcast when all assets have been successfully loaded.
DECLARE_DELEGATE(FOnAssetsLoaded);

/**
 * Manages asynchronous loading of all UI-related assets.
 */
UCLASS()
class STRAFEUI_API US_UI_AssetManager : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Initializes the asset manager with the necessary settings.
     * @param InSettings The UI settings containing asset references.
     */
    void Initialize(const US_UI_Settings* InSettings);

    /** Starts the asynchronous asset loading process. */
    void StartAssetsLoading();

    /** Checks if all primary assets have been loaded. */
    bool AreAssetsLoaded() const { return bAssetsLoaded; }

    /**
     * Retrieves a cached screen widget class.
     * @param ScreenId The ID of the screen to retrieve.
     * @return The widget class, or nullptr if not found or not loaded.
     */
    TSubclassOf<UCommonActivatableWidget> GetScreenWidgetClass(E_UIScreenId ScreenId) const;

    /** Delegate broadcast when asset loading is complete. */
    FOnAssetsLoaded OnAssetsLoaded;

private:
    /** Callback for when the initial ScreenMapDataAsset is loaded. */
    void OnScreenMapDataAssetLoaded(FSoftObjectPath ScreenDataAssetPath);

    /** Final callback for when all queued UI assets are loaded. */
    void OnAllAssetsLoaded();

    /** A weak pointer to the UI settings asset. */
    UPROPERTY()
    TWeakObjectPtr<const US_UI_Settings> UISettings;

    /** A cache of screen widget classes, populated after async loading. */
    UPROPERTY()
    TMap<E_UIScreenId, TSubclassOf<UCommonActivatableWidget>> ScreenWidgetClassCache;

    /** Handle for the main asynchronous loading operation. */
    TSharedPtr<FStreamableHandle> AllAssetsHandle;

    bool bAssetsLoaded = false;
    bool bAreAssetsLoading = false;
};