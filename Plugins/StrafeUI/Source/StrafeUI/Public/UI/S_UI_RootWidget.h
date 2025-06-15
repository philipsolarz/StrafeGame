// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_RootWidget.h

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Components/Border.h"
#include "Components/NamedSlot.h" // Required for UNamedSlot
#include "S_UI_RootWidget.generated.h"

/**
 * @class S_UI_RootWidget
 * @brief The C++ base for WBP_UIRoot, the main container for the game's UI.
 * Manages the primary screen stack and global overlays.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_RootWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    /** Returns the widget stack where content screens are displayed. */
    UCommonActivatableWidgetStack* GetContentStack() const { return MainContentStack; }

    /** Returns the named slot where the main menu is placed. */
    UNamedSlot* GetMainMenuSlot() const { return MainMenu; }

private:
    /** The named slot that will hold the persistent Main Menu widget. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UNamedSlot> MainMenu;

    /** The widget stack where all primary content screens (e.g., Settings, Find Game) are pushed. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonActivatableWidgetStack> MainContentStack;

    /** An overlay that can be used for global UI effects like fade-in/fade-out transitions. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> Overlay_Transition;
};