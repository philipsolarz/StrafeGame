#include "UI/Widgets/S_WeaponInventoryListWidget.h"
#include "UI/ViewModels/S_PlayerHUDViewModel.h"
#include "UI/ViewModels/S_WeaponViewModel.h" // For casting/type safety
#include "Components/ListView.h"

void US_WeaponInventoryListWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // Initial refresh can happen here if ViewModel is set, or wait for SetPlayerHUDViewModel
}

void US_WeaponInventoryListWidget::NativeDestruct()
{
    if (PlayerHUDViewModel)
    {
        PlayerHUDViewModel->OnViewModelUpdated.RemoveDynamic(this, &US_WeaponInventoryListWidget::HandlePlayerHUDViewModelUpdated);
    }
    Super::NativeDestruct();
}

void US_WeaponInventoryListWidget::SetPlayerHUDViewModel(US_PlayerHUDViewModel* InViewModel)
{
    if (PlayerHUDViewModel)
    {
        // Unbind from old ViewModel if any
        PlayerHUDViewModel->OnViewModelUpdated.RemoveDynamic(this, &US_WeaponInventoryListWidget::HandlePlayerHUDViewModelUpdated);
    }

    PlayerHUDViewModel = InViewModel;

    if (PlayerHUDViewModel)
    {
        PlayerHUDViewModel->OnViewModelUpdated.AddUniqueDynamic(this, &US_WeaponInventoryListWidget::HandlePlayerHUDViewModelUpdated);
        RefreshInventoryList(); // Initial population
    }
    else
    {
        if (InventoryListView)
        {
            InventoryListView->ClearListItems();
        }
    }
}

void US_WeaponInventoryListWidget::HandlePlayerHUDViewModelUpdated()
{
    RefreshInventoryList();
}

void US_WeaponInventoryListWidget::RefreshInventoryList()
{
    if (!PlayerHUDViewModel || !InventoryListView)
    {
        if (InventoryListView) InventoryListView->ClearListItems(); // Clear if VM is null
        return;
    }

    InventoryListView->ClearListItems();

    for (US_WeaponViewModel* WeaponVM : PlayerHUDViewModel->WeaponInventoryViewModels)
    {
        if (WeaponVM)
        {
            InventoryListView->AddItem(WeaponVM);
        }
    }
}