#include "S_UI_AssetManager.h"
#include "S_UI_Settings.h"
#include "Engine/AssetManager.h"
#include "Data/S_UI_ScreenDataAsset.h"

void US_UI_AssetManager::Initialize(const US_UI_Settings* InSettings)
{
    UISettings = InSettings;
}

void US_UI_AssetManager::StartAssetsLoading()
{
    if (bAssetsLoaded || bAreAssetsLoading)
    {
        return;
    }
    bAreAssetsLoading = true;

    UE_LOG(LogTemp, Log, TEXT("S_UI_AssetManager: Starting asynchronous asset loading..."));

    if (!UISettings.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("S_UI_AssetManager: Cannot find StrafeUISettings!"));
        bAreAssetsLoading = false;
        return;
    }

    FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
    FSoftObjectPath ScreenDataAssetPath = UISettings->ScreenMapDataAsset.ToSoftObjectPath();

    TWeakObjectPtr<US_UI_AssetManager> WeakThis = this;
    StreamableManager.RequestAsyncLoad(ScreenDataAssetPath,
        [WeakThis, ScreenDataAssetPath]()
        {
            if (WeakThis.IsValid())
            {
                WeakThis->OnScreenMapDataAssetLoaded(ScreenDataAssetPath);
            }
        });
}

void US_UI_AssetManager::OnScreenMapDataAssetLoaded(FSoftObjectPath ScreenDataAssetPath)
{
    UE_LOG(LogTemp, Log, TEXT("S_UI_AssetManager: Screen map data asset loaded."));

    if (!UISettings.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("S_UI_AssetManager: Settings became invalid during load."));
        bAreAssetsLoading = false;
        return;
    }

    US_UI_ScreenDataAsset* ScreenData = Cast<US_UI_ScreenDataAsset>(UISettings->ScreenMapDataAsset.Get());

    if (!ScreenData)
    {
        UE_LOG(LogTemp, Error, TEXT("S_UI_AssetManager: Failed to load ScreenMapDataAsset after async load!"));
        bAreAssetsLoading = false;
        return;
    }

    TArray<FSoftObjectPath> AssetsToLoad;
    AssetsToLoad.Add(UISettings->RootWidgetClass.ToSoftObjectPath());
    AssetsToLoad.Add(UISettings->MainMenuWidgetClass.ToSoftObjectPath());
    AssetsToLoad.Add(UISettings->ModalStackClass.ToSoftObjectPath());
    AssetsToLoad.Add(UISettings->ModalWidgetClass.ToSoftObjectPath());
    AssetsToLoad.Add(UISettings->InputControllerClass.ToSoftObjectPath());
    AssetsToLoad.Add(UISettings->TabButtonClass.ToSoftObjectPath());
    AssetsToLoad.Add(UISettings->AudioSettingsTabClass.ToSoftObjectPath());
    AssetsToLoad.Add(UISettings->VideoSettingsTabClass.ToSoftObjectPath());
    AssetsToLoad.Add(UISettings->ControlsSettingsTabClass.ToSoftObjectPath());
    AssetsToLoad.Add(UISettings->GameplaySettingsTabClass.ToSoftObjectPath());
    AssetsToLoad.Add(UISettings->PlayerSettingsTabClass.ToSoftObjectPath());

    // --- FIX: ADD PAUSE MENU WIDGET TO ASSET LOADING ---
    AssetsToLoad.Add(UISettings->PauseMenuWidgetClass.ToSoftObjectPath());
    // --- END FIX ---

    for (const F_UIScreenDefinition& Definition : ScreenData->ScreenDefinitions)
    {
        if (!Definition.WidgetClass.IsNull())
        {
            AssetsToLoad.Add(Definition.WidgetClass.ToSoftObjectPath());
        }
    }

    FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
    TWeakObjectPtr<US_UI_AssetManager> WeakThis = this;
    AllAssetsHandle = StreamableManager.RequestAsyncLoad(AssetsToLoad,
        [WeakThis]()
        {
            if (WeakThis.IsValid())
            {
                WeakThis->OnAllAssetsLoaded();
            }
        });
}

void US_UI_AssetManager::OnAllAssetsLoaded()
{
    UE_LOG(LogTemp, Log, TEXT("S_UI_AssetManager: All UI assets finished loading."));

    if (!UISettings.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("S_UI_AssetManager: Settings are null in OnAllAssetsLoaded."));
        bAreAssetsLoading = false;
        return;
    }

    const US_UI_ScreenDataAsset* ScreenData = Cast<US_UI_ScreenDataAsset>(UISettings->ScreenMapDataAsset.Get());
    if (ScreenData)
    {
        for (const F_UIScreenDefinition& Definition : ScreenData->ScreenDefinitions)
        {
            if (UClass* LoadedClass = Definition.WidgetClass.Get())
            {
                ScreenWidgetClassCache.Add(Definition.ScreenId, LoadedClass);
                UE_LOG(LogTemp, Log, TEXT("Cached screen %s -> %s"), *UEnum::GetValueAsString(Definition.ScreenId), *LoadedClass->GetName());
            }
        }
    }

    bAssetsLoaded = true;
    bAreAssetsLoading = false;

    // Broadcast that loading is complete.
    OnAssetsLoaded.ExecuteIfBound();
}

TSubclassOf<UCommonActivatableWidget> US_UI_AssetManager::GetScreenWidgetClass(E_UIScreenId ScreenId) const
{
    if (const TSubclassOf<UCommonActivatableWidget>* FoundClass = ScreenWidgetClassCache.Find(ScreenId))
    {
        return *FoundClass;
    }
    return nullptr;
}