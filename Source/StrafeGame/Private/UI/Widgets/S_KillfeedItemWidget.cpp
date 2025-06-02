#include "UI/Widgets/S_KillfeedItemWidget.h"
#include "UI/ViewModels/S_KillfeedItemViewModel.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void US_KillfeedItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    // IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject); // Call if you override the BP event and want parent functionality

    ItemViewModel = Cast<US_KillfeedItemViewModel>(ListItemObject);
    RefreshDisplay();
}

void US_KillfeedItemWidget::RefreshDisplay()
{
    if (!ItemViewModel)
    {
        if (TxtKillerName) TxtKillerName->SetText(FText::GetEmpty());
        if (TxtVictimName) TxtVictimName->SetText(FText::GetEmpty());
        if (ImgKillWeaponIcon) ImgKillWeaponIcon->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    if (TxtKillerName)
    {
        TxtKillerName->SetText(FText::FromString(ItemViewModel->KillerName));
        // Example: FLinearColor KillerColor = ItemViewModel->bIsLocalPlayerKiller ? FLinearColor::Green : FLinearColor::White;
        // TxtKillerName->SetColorAndOpacity(KillerColor);
    }
    if (TxtVictimName)
    {
        TxtVictimName->SetText(FText::FromString(ItemViewModel->VictimName));
        // Example: FLinearColor VictimColor = ItemViewModel->bIsLocalPlayerVictim ? FLinearColor::Red : FLinearColor::White;
        // TxtVictimName->SetColorAndOpacity(VictimColor);
    }
    if (ImgKillWeaponIcon)
    {
        if (ItemViewModel->WeaponIcon.IsValid())
        {
            UTexture2D* IconTex = ItemViewModel->WeaponIcon.LoadSynchronous();
            if (IconTex)
            {
                ImgKillWeaponIcon->SetBrushFromTexture(IconTex);
                ImgKillWeaponIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
            }
            else
            {
                ImgKillWeaponIcon->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
        else
        {
            ImgKillWeaponIcon->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}