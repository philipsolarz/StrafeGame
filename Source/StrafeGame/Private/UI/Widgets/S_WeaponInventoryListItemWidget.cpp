#include "UI/Widgets/S_WeaponInventoryListItemWidget.h"
#include "UI/ViewModels/S_WeaponViewModel.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Engine/Texture2D.h" // For TSoftObjectPtr resolution

void US_WeaponInventoryListItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    // This function is called by the ListView when it creates this widget an_d assigns it an item.
    // The ListItemObject is the US_WeaponViewModel* we added to the ListView.
    ItemViewModel = Cast<US_WeaponViewModel>(ListItemObject);

    if (ItemViewModel)
    {
        // Subscribe to the ViewModel's update delegate
        ItemViewModel->OnWeaponViewModelUpdated.AddUniqueDynamic(this, &US_WeaponInventoryListItemWidget::RefreshItemDisplay);
        RefreshItemDisplay(); // Initial display update
    }
    else
    {
        // Clear display if ViewModel is null (optional, depends on desired behavior)
        if (TxtWeaponName) TxtWeaponName->SetText(FText::GetEmpty());
        if (TxtAmmoCount) TxtAmmoCount->SetText(FText::GetEmpty());
        if (ImgWeaponIcon) ImgWeaponIcon->SetBrush(FSlateBrush());
        if (HighlightIndicator) HighlightIndicator->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void US_WeaponInventoryListItemWidget::RefreshItemDisplay()
{
    if (!ItemViewModel)
    {
        return;
    }

    if (TxtWeaponName)
    {
        TxtWeaponName->SetText(ItemViewModel->WeaponName);
    }

    if (TxtAmmoCount)
    {
        FText AmmoText = FText::Format(
            NSLOCTEXT("WeaponInventory", "AmmoFormat", "{0} / {1}"),
            ItemViewModel->CurrentAmmo,
            ItemViewModel->MaxAmmo
        );
        TxtAmmoCount->SetText(AmmoText);
    }

    if (ImgWeaponIcon)
    {
        if (ItemViewModel->WeaponIcon.IsValid()) // Check if soft pointer is set
        {
            // Attempt to load synchronously for simplicity in list items.
            // For many items or large icons, consider async loading managed by the ViewModel or a texture manager.
            UTexture2D* IconTexture = ItemViewModel->WeaponIcon.LoadSynchronous();
            if (IconTexture)
            {
                ImgWeaponIcon->SetBrushFromTexture(IconTexture);
            }
            else
            {
                ImgWeaponIcon->SetBrush(FSlateBrush()); // Clear or set default
            }
        }
        else
        {
            ImgWeaponIcon->SetBrush(FSlateBrush()); // Clear or set default
        }
    }

    if (HighlightIndicator)
    {
        HighlightIndicator->SetVisibility(ItemViewModel->bIsEquipped ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    }
}