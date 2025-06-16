#include "UI/S_UI_FindGameWidget.h"
#include "UI/S_UI_ServerFilterWidget.h"
#include "ViewModel/S_UI_VM_ServerBrowser.h"
#include "CommonButtonBase.h"
#include "UI/S_UI_CollapsibleBox.h"
#include "S_UI_Settings.h"
#include "Components/ListView.h"
#include "Components/CheckBox.h"
#include "S_UI_Subsystem.h"
#include "S_UI_Navigator.h"

US_UI_ViewModelBase* US_UI_FindGameWidget::CreateViewModel()
{
    US_UI_VM_ServerBrowser* VM = NewObject<US_UI_VM_ServerBrowser>(this);
    VM->Initialize();
    return VM;
}

void US_UI_FindGameWidget::SetViewModel(US_UI_ViewModelBase* InViewModel)
{
    if (US_UI_VM_ServerBrowser* InServerBrowserViewModel = Cast<US_UI_VM_ServerBrowser>(InViewModel))
    {
        ViewModel = InServerBrowserViewModel;

        // Bind to the ViewModel's OnDataChanged delegate to be notified of updates.
        ViewModel->OnDataChanged.AddUniqueDynamic(this, &US_UI_FindGameWidget::OnServerListUpdated);

        // Bind the refresh button click now that the ViewModel is valid.
        if (Btn_Refresh)
        {
            // Clear any previous bindings to be safe
            Btn_Refresh->OnClicked().Clear();
            Btn_Refresh->OnClicked().AddUObject(ViewModel.Get(), &US_UI_VM_ServerBrowser::RequestServerListRefresh);
        }

        // Trigger an initial server list refresh when the screen is opened.
        ViewModel->RequestServerListRefresh();
    }
}

void US_UI_FindGameWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Chk_SearchLAN)
    {
        Chk_SearchLAN->OnCheckStateChanged.AddDynamic(this, &US_UI_FindGameWidget::HandleSearchLANChanged);
    }

    if (Btn_Join)
    {
        Btn_Join->OnClicked().AddUObject(this, &US_UI_FindGameWidget::HandleJoinClicked);
    }
    if (Btn_Back)
    {
        Btn_Back->OnClicked().AddUObject(this, &US_UI_FindGameWidget::HandleBackClicked);
    }

    if (List_Servers)
    {
        List_Servers->OnItemSelectionChanged().AddUObject(this, &US_UI_FindGameWidget::OnServerSelected);
    }

    // Bind to filter widget changes
    if (ServerFilterWidget)
    {
        ServerFilterWidget->OnFiltersChanged.AddDynamic(this, &US_UI_FindGameWidget::OnFiltersChanged);
    }

    // Setup the collapsible box header
    if (Col_Filters)
    {
        Col_Filters->HeaderText = FText::FromString(TEXT("Filters"));
    }
}

void US_UI_FindGameWidget::OnServerListUpdated()
{
    if (ViewModel.IsValid() && List_Servers)
    {
        // Store the currently selected item before clearing
        US_UI_VM_ServerListEntry* PreviouslySelected = List_Servers->GetSelectedItem<US_UI_VM_ServerListEntry>();
        FString PreviousServerName;
        if (PreviouslySelected)
        {
            PreviousServerName = PreviouslySelected->ServerInfo.ServerName.ToString();
        }

        List_Servers->ClearListItems();

        // Populate the list view with data from the ViewModel.
        for (const F_ServerInfo& ServerInfo : ViewModel->ServerList)
        {
            // Create a UObject wrapper for our list entry data
            US_UI_VM_ServerListEntry* Entry = NewObject<US_UI_VM_ServerListEntry>(this);
            Entry->ServerInfo = ServerInfo;

            // Find the corresponding full search result from the ViewModel's internal list
            // This is needed for the join functionality
            for (const auto& FoundServer : ViewModel->AllFoundServers)
            {
                if (FoundServer &&
                    FoundServer->ServerInfo.ServerName.EqualTo(ServerInfo.ServerName) &&
                    FoundServer->ServerInfo.Ping == ServerInfo.Ping)
                {
                    Entry->SessionSearchResult = FoundServer->SessionSearchResult;
                    break;
                }
            }

            // Add the data object to the list view. The list view will create a widget for it.
            List_Servers->AddItem(Entry);

            // Try to restore selection if this was the previously selected server
            if (!PreviousServerName.IsEmpty() && Entry->ServerInfo.ServerName.ToString() == PreviousServerName)
            {
                List_Servers->SetSelectedItem(Entry);
            }
        }

        // Update button states
        UpdateButtonStates();
    }
}

void US_UI_FindGameWidget::OnServerSelected(UObject* Item)
{
    // The only thing we need to do when selection changes is update the button states.
    UpdateButtonStates();
}


void US_UI_FindGameWidget::HandleJoinClicked()
{
    if (!ViewModel.IsValid() || !List_Servers)
    {
        return;
    }

    // Get the selected server entry
    US_UI_VM_ServerListEntry* SelectedItem = List_Servers->GetSelectedItem<US_UI_VM_ServerListEntry>();
    if (!SelectedItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("No server selected"));

        // Show a modal asking the user to select a server
        if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
        {
            F_UIModalPayload Payload;
            Payload.Message = FText::FromString(TEXT("Please select a server to join."));
            Payload.ModalType = E_UIModalType::OK;
            UISubsystem->RequestModal(Payload, FOnModalDismissedSignature());
        }
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Attempting to join server: %s"), *SelectedItem->ServerInfo.ServerName.ToString());

    // Check if the server is full
    if (SelectedItem->ServerInfo.PlayerCount >= SelectedItem->ServerInfo.MaxPlayers)
    {
        // Ask for confirmation if server is full
        if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
        {
            F_UIModalPayload Payload;
            Payload.Message = FText::FromString(TEXT("This server appears to be full. Do you still want to try joining?"));
            Payload.ModalType = E_UIModalType::YesNo;

            // Capture the selected item in the lambda
            UISubsystem->RequestModal(Payload, FOnModalDismissedSignature::CreateLambda(
                [this, SelectedItem](bool bConfirmed)
                {
                    if (bConfirmed && ViewModel.IsValid())
                    {
                        ViewModel->JoinSession(SelectedItem->SessionSearchResult);
                    }
                }));
        }
    }
    else
    {
        // Join directly if server has space
        ViewModel->JoinSession(SelectedItem->SessionSearchResult);
    }
}

void US_UI_FindGameWidget::HandleBackClicked()
{
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        UISubsystem->GetNavigator()->PopContentScreen();
    }
}

void US_UI_FindGameWidget::UpdateButtonStates()
{
    if (Btn_Join && List_Servers)
    {
        // Enable/disable join button based on selection
        bool bHasSelection = List_Servers->GetSelectedItem() != nullptr;
        Btn_Join->SetIsEnabled(bHasSelection);
    }
}

void US_UI_FindGameWidget::OnFiltersChanged()
{
    if (!ViewModel.IsValid() || !ServerFilterWidget)
    {
        return;
    }

    // Update the view model's filter properties
    ViewModel->FilterServerName = ServerFilterWidget->GetServerNameFilter();
    ViewModel->FilterGameMode = ServerFilterWidget->GetGameModeFilter();
    ViewModel->bFilterHideFullServers = ServerFilterWidget->GetHideFullServers();
    ViewModel->bFilterHideEmptyServers = ServerFilterWidget->GetHideEmptyServers();
    ViewModel->bFilterHidePrivateServers = ServerFilterWidget->GetHidePrivateServers();
    ViewModel->FilterMaxPing = ServerFilterWidget->GetMaxPing();

    // Apply the filters
    ViewModel->ApplyFilters();
}

void US_UI_FindGameWidget::HandleSearchLANChanged(bool bIsChecked)
{
    if (ViewModel.IsValid())
    {
        ViewModel->bSearchLAN = bIsChecked;
    }
}