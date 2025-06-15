// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_ReplaysWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "S_UI_ReplaysWidget.generated.h"

class US_UI_VM_Replays;
class UListView;
class UCommonButtonBase;
class UThrobber;

/**
 * Widget for managing replay files
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_ReplaysWidget : public US_UI_BaseScreenWidget
{
    GENERATED_BODY()

public:
    /** Sets the ViewModel for this widget */
    void SetViewModel(US_UI_ViewModelBase* InViewModel);

    virtual US_UI_ViewModelBase* CreateViewModel() override;

protected:
    virtual void NativeOnInitialized() override;

private:
    /** Called when the ViewModel's data has changed */
    UFUNCTION()
    void OnViewModelDataChanged();

    /** Called when a replay is selected in the list */
    UFUNCTION()
    void OnReplaySelectionChanged(UObject* SelectedItem);

    /** Called when the refresh button is clicked */
    UFUNCTION()
    void OnRefreshClicked();

    /** Called when the play button is clicked */
    UFUNCTION()
    void OnPlayClicked();

    /** Called when the delete button is clicked */
    UFUNCTION()
    void OnDeleteClicked();

    /** Called when the back button is clicked */
    UFUNCTION()
    void OnBackClicked();

    /** Updates button states based on selection */
    void UpdateButtonStates();

    /** The ViewModel that provides data for this widget */
    UPROPERTY()
    TWeakObjectPtr<US_UI_VM_Replays> ViewModel;

    // UI Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UListView> ListView_Replays;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Refresh;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Play;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Delete;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Back;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UThrobber> Throbber_Loading;
};