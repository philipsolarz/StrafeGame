// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_FindGameWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "ViewModel/S_UI_VM_ServerBrowser.h"
#include "S_UI_FindGameWidget.generated.h"

class UCommonButtonBase;
class UListView;
class UCheckBox; // Keep this forward declaration
class US_UI_CollapsibleBox;
class US_UI_ServerFilterWidget;

/**
 * @class S_UI_FindGameWidget
 * @brief The C++ base for the Find Game screen.
 * Binds to a ViewModel to display a list of servers.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_FindGameWidget : public US_UI_BaseScreenWidget
{
    GENERATED_BODY()

public:
    /** Sets the ViewModel for this widget and triggers the initial UI update. */
    void SetViewModel(US_UI_ViewModelBase* InViewModel);

    virtual US_UI_ViewModelBase* CreateViewModel() override;

protected:
    virtual void NativeOnInitialized() override;

private:
    /** Called when the ViewModel's data has changed, refreshing the UI. */
    UFUNCTION()
    void OnServerListUpdated();

    /** Called when the user clicks on an item in the server list. */
    UFUNCTION()
    void OnServerSelected(UObject* Item);

    /** Updates the enabled state of buttons based on current selection */
    void UpdateButtonStates();

    /** Called when filter values change */
    UFUNCTION()
    void OnFiltersChanged();

    /** Called when the Search LAN checkbox is changed */
    UFUNCTION()
    void HandleSearchLANChanged(bool bIsChecked);

    //~ Button Click Handlers
    UFUNCTION()
    void HandleJoinClicked();

    UFUNCTION()
    void HandleBackClicked();

    /** The ViewModel that provides data for this widget. */
    UPROPERTY()
    TWeakObjectPtr<US_UI_VM_ServerBrowser> ViewModel;

    //~ UPROPERTY Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UListView> List_Servers;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Refresh;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Join;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Back;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<US_UI_CollapsibleBox> Col_Filters;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<US_UI_ServerFilterWidget> ServerFilterWidget;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCheckBox> Chk_SearchLAN;
};