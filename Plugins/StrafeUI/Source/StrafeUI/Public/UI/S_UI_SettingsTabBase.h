// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_SettingsTabBase.h

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "S_UI_SettingsTabBase.generated.h"

class US_UI_VM_Settings;

/**
 * Base class for individual settings tab content panels.
 * Each settings category (Audio, Video, etc.) should inherit from this.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_SettingsTabBase : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    /**
     * Sets the settings view model for this tab.
     * @param InViewModel The settings view model
     */
    virtual void SetViewModel(US_UI_VM_Settings* InViewModel);

    /**
     * Called when settings in this tab should be applied.
     * Override in derived classes to handle specific settings.
     */
    virtual void ApplySettings() {}

    /**
     * Called when settings in this tab should be reverted.
     * Override in derived classes to handle specific settings.
     */
    virtual void RevertSettings() {}

    /**
     * Called to check if this tab has unsaved changes.
     * @return True if there are unsaved changes
     */
    virtual bool HasUnsavedChanges() const { return false; }

protected:
    /** The settings view model shared across all tabs. */
    UPROPERTY()
    TWeakObjectPtr<US_UI_VM_Settings> ViewModel;

    /**
     * Called when the view model data changes.
     * Override to update UI elements.
     */
    UFUNCTION()
    virtual void OnViewModelDataChanged() {}
};