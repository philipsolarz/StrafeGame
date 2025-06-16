#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Data/S_UI_ScreenTypes.h"
#include "S_UI_Navigator.generated.h"

class US_UI_RootWidget;
class US_UI_AssetManager;
class IViewModelProvider;
class US_UI_ViewModelBase;

/**
 * Manages UI screen transitions, including switching and popping screens.
 */
UCLASS()
class STRAFEUI_API US_UI_Navigator : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Initializes the navigator with the required root widget and asset manager.
     * @param InRootWidget The root widget containing the content stack.
     * @param InAssetManager The asset manager for retrieving widget classes.
     */
    void Initialize(US_UI_RootWidget* InRootWidget, US_UI_AssetManager* InAssetManager);

    /** Switches the main content area to a new screen. */
    void SwitchContentScreen(E_UIScreenId ScreenId);

    /** Pops the current screen from the content stack. */
    void PopContentScreen();

private:
    /** A weak pointer to the root UI widget. */
    UPROPERTY()
    TWeakObjectPtr<US_UI_RootWidget> UIRootWidget;

    /** A weak pointer to the asset manager. */
    UPROPERTY()
    TWeakObjectPtr<US_UI_AssetManager> AssetManager;

    /** If a screen switch is requested before assets are loaded, it's stored here. */
    E_UIScreenId PendingScreenRequest = E_UIScreenId::None;
};