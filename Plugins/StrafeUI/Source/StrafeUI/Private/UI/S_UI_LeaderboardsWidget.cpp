// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_LeaderboardsWidget.cpp

#include "UI/S_UI_LeaderboardsWidget.h"
#include "ViewModel/S_UI_VM_Leaderboards.h"
#include "Components/ComboBoxString.h"
#include "Components/ListView.h"
#include "Components/Throbber.h"
#include "CommonButtonBase.h"
#include "S_UI_Subsystem.h"
#include "S_UI_Navigator.h"

US_UI_ViewModelBase* US_UI_LeaderboardsWidget::CreateViewModel()
{
    US_UI_VM_Leaderboards* VM = NewObject<US_UI_VM_Leaderboards>(this);
    VM->Initialize();
    return VM;
}

void US_UI_LeaderboardsWidget::SetViewModel(US_UI_ViewModelBase* InViewModel)
{
    if (US_UI_VM_Leaderboards* InLeaderboardsViewModel = Cast<US_UI_VM_Leaderboards>(InViewModel))
    {
        ViewModel = InLeaderboardsViewModel;

        // Bind to data changes
        ViewModel->OnDataChanged.AddUniqueDynamic(this, &US_UI_LeaderboardsWidget::OnViewModelDataChanged);

        // Initial update
        OnViewModelDataChanged();
    }
}

void US_UI_LeaderboardsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Bind combo box selection change
    if (ComboBox_MapFilter)
    {
        ComboBox_MapFilter->OnSelectionChanged.AddDynamic(this, &US_UI_LeaderboardsWidget::OnMapFilterChanged);
    }

    // Bind button clicks
    if (Btn_Refresh)
    {
        Btn_Refresh->OnClicked().AddUObject(this, &US_UI_LeaderboardsWidget::OnRefreshClicked);
    }

    if (Btn_Back)
    {
        Btn_Back->OnClicked().AddUObject(this, &US_UI_LeaderboardsWidget::OnBackClicked);
    }
}

void US_UI_LeaderboardsWidget::OnViewModelDataChanged()
{
    if (!ViewModel.IsValid())
    {
        return;
    }

    // Update map filter combo box
    if (ComboBox_MapFilter)
    {
        // Only repopulate if the options have changed
        if (ComboBox_MapFilter->GetOptionCount() != ViewModel->MapNames.Num())
        {
            ComboBox_MapFilter->ClearOptions();
            for (const FString& MapName : ViewModel->MapNames)
            {
                ComboBox_MapFilter->AddOption(MapName);
            }
        }

        // Update selection
        if (!ViewModel->CurrentMapName.IsEmpty())
        {
            ComboBox_MapFilter->SetSelectedOption(ViewModel->CurrentMapName);
        }
    }

    // Update loading indicator
    if (Throbber_Loading)
    {
        Throbber_Loading->SetVisibility(ViewModel->bIsLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    // Update leaderboard list
    if (ListView_Leaderboard)
    {
        ListView_Leaderboard->ClearListItems();
        for (UObject* Entry : ViewModel->LeaderboardEntries)
        {
            ListView_Leaderboard->AddItem(Entry);
        }
    }
}

void US_UI_LeaderboardsWidget::OnMapFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (ViewModel.IsValid() && SelectionType != ESelectInfo::Direct)
    {
        ViewModel->SetMapFilter(SelectedItem);
    }
}

void US_UI_LeaderboardsWidget::OnRefreshClicked()
{
    if (ViewModel.IsValid())
    {
        ViewModel->RefreshLeaderboard();
    }
}

void US_UI_LeaderboardsWidget::OnBackClicked()
{
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        UISubsystem->GetNavigator()->PopContentScreen();
    }
}