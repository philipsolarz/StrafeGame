// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_LeaderboardsWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "S_UI_LeaderboardsWidget.generated.h"

class US_UI_VM_Leaderboards;
class UComboBoxString;
class UListView;
class UThrobber;
class UCommonButtonBase;

/**
 * Widget for displaying game leaderboards
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_LeaderboardsWidget : public US_UI_BaseScreenWidget
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

    /** Called when the map filter selection changes */
    UFUNCTION()
    void OnMapFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    /** Called when the refresh button is clicked */
    UFUNCTION()
    void OnRefreshClicked();

    /** Called when the back button is clicked */
    UFUNCTION()
    void OnBackClicked();

    /** The ViewModel that provides data for this widget */
    UPROPERTY()
    TWeakObjectPtr<US_UI_VM_Leaderboards> ViewModel;

    // UI Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UComboBoxString> ComboBox_MapFilter;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UListView> ListView_Leaderboard;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UThrobber> Throbber_Loading;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Refresh;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Back;
};