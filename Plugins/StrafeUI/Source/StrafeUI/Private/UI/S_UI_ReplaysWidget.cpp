// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_ReplaysWidget.cpp

#include "UI/S_UI_ReplaysWidget.h"
#include "ViewModel/S_UI_VM_Replays.h"
#include "Components/ListView.h"
#include "Components/Throbber.h"
#include "CommonButtonBase.h"
#include "S_UI_Subsystem.h"
#include "S_UI_Navigator.h"
#include "GameFramework/PlayerController.h"

US_UI_ViewModelBase* US_UI_ReplaysWidget::CreateViewModel()
{
    US_UI_VM_Replays* VM = NewObject<US_UI_VM_Replays>(this);
    VM->Initialize();
    return VM;
}

void US_UI_ReplaysWidget::SetViewModel(US_UI_ViewModelBase* InViewModel)
{
    if (US_UI_VM_Replays* InReplaysViewModel = Cast<US_UI_VM_Replays>(InViewModel))
    {
        ViewModel = InReplaysViewModel;

        // Bind to data changes
        ViewModel->OnDataChanged.AddUniqueDynamic(this, &US_UI_ReplaysWidget::OnViewModelDataChanged);

        // Initial update
        OnViewModelDataChanged();
    }
}

void US_UI_ReplaysWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Bind list view selection change
    if (ListView_Replays)
    {
        ListView_Replays->OnItemSelectionChanged().AddUObject(this, &US_UI_ReplaysWidget::OnReplaySelectionChanged);
    }

    // Bind button clicks
    if (Btn_Refresh)
    {
        Btn_Refresh->OnClicked().AddUObject(this, &US_UI_ReplaysWidget::OnRefreshClicked);
    }

    if (Btn_Play)
    {
        Btn_Play->OnClicked().AddUObject(this, &US_UI_ReplaysWidget::OnPlayClicked);
    }

    if (Btn_Delete)
    {
        Btn_Delete->OnClicked().AddUObject(this, &US_UI_ReplaysWidget::OnDeleteClicked);
    }

    if (Btn_Back)
    {
        Btn_Back->OnClicked().AddUObject(this, &US_UI_ReplaysWidget::OnBackClicked);
    }
}

void US_UI_ReplaysWidget::OnViewModelDataChanged()
{
    if (!ViewModel.IsValid())
    {
        return;
    }

    // Update loading indicator
    if (Throbber_Loading)
    {
        Throbber_Loading->SetVisibility(ViewModel->bIsLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    // Update replay list
    if (ListView_Replays)
    {
        ListView_Replays->ClearListItems();
        for (UObject* Entry : ViewModel->ReplayEntries)
        {
            ListView_Replays->AddItem(Entry);
        }

        // Restore selection if needed
        if (ViewModel->SelectedReplay)
        {
            ListView_Replays->SetSelectedItem(ViewModel->SelectedReplay);
        }
    }

    // Update button states
    UpdateButtonStates();
}

void US_UI_ReplaysWidget::OnReplaySelectionChanged(UObject* SelectedItem)
{
    if (ViewModel.IsValid())
    {
        ViewModel->SetSelectedReplay(SelectedItem);
    }
}

void US_UI_ReplaysWidget::OnRefreshClicked()
{
    if (ViewModel.IsValid())
    {
        ViewModel->RefreshReplays();
    }
}

void US_UI_ReplaysWidget::OnPlayClicked()
{
    if (ViewModel.IsValid() && GetOwningPlayer())
    {
        ViewModel->PlaySelectedReplay(GetOwningPlayer());
    }
}

void US_UI_ReplaysWidget::OnDeleteClicked()
{
    if (ViewModel.IsValid())
    {
        ViewModel->DeleteSelectedReplay();
    }
}

void US_UI_ReplaysWidget::OnBackClicked()
{
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        UISubsystem->GetNavigator()->PopContentScreen();
    }
}

void US_UI_ReplaysWidget::UpdateButtonStates()
{
    bool bHasSelection = ViewModel.IsValid() && ViewModel->SelectedReplay != nullptr;
    bool bIsOperationInProgress = ViewModel.IsValid() && (ViewModel->bIsLoading || ViewModel->bIsDeleting);

    if (Btn_Play)
    {
        Btn_Play->SetIsEnabled(bHasSelection && !bIsOperationInProgress);
    }

    if (Btn_Delete)
    {
        Btn_Delete->SetIsEnabled(bHasSelection && !bIsOperationInProgress);
    }

    if (Btn_Refresh)
    {
        Btn_Refresh->SetIsEnabled(!bIsOperationInProgress);
    }
}