// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_SettingsWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "ViewModel/S_UI_VM_Settings.h"
#include "S_UI_SettingsWidget.generated.h"

class UCommonButtonBase;
class US_UI_TabControl;
class US_UI_SettingsTabBase;

/**
 * @class S_UI_SettingsWidget
 * @brief The C++ base for the Settings screen with tabbed interface.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_SettingsWidget : public US_UI_BaseScreenWidget
{
    GENERATED_BODY()

public:
    /**
     * Sets the ViewModel for this widget. This is the entry point for initializing
     * the settings tabs with their content and data context.
     */
    void SetViewModel(US_UI_ViewModelBase* InViewModel);

    virtual US_UI_ViewModelBase* CreateViewModel() override;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeDestruct() override;

private:
    /** Initializes the tab control with settings categories defined in US_UI_Settings. */
    void InitializeSettingsTabs();

    /** Called via delegate from the TabControl when all tab assets have finished loading. */
    UFUNCTION()
    void HandleTabsInitialized();

    /** Called when a settings tab is selected. */
    UFUNCTION()
    void OnSettingsTabSelected(int32 TabIndex, FName TabTag);

    //~ Button Click Handlers
    UFUNCTION()
    void HandleApplyClicked();

    UFUNCTION()
    void HandleRevertClicked();

    UFUNCTION()
    void HandleBackClicked();

    /** The ViewModel that provides data for this widget. */
    UPROPERTY()
    TWeakObjectPtr<US_UI_VM_Settings> ViewModel;

    //~ UPROPERTY Bindings for UI elements
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<US_UI_TabControl> TabControl;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Apply;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Revert;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Back;

    /** Array of all settings tab content widgets. */
    UPROPERTY()
    TArray<US_UI_SettingsTabBase*> SettingsTabs;

    /** Flag to ensure tabs are initialized only once. */
    bool bTabsInitialized = false;
};