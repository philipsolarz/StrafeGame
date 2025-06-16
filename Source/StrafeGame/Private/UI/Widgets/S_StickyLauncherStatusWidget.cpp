#include "UI/Widgets/S_StickyLauncherStatusWidget.h"
#include "UI/ViewModels/S_StickyGrenadeLauncherViewModel.h"
#include "Components/Image.h" // Include UImage for visibility control

void US_StickyLauncherStatusWidget::SetViewModel(US_StickyGrenadeLauncherViewModel* InViewModel)
{
    if (StickyLauncherViewModel)
    {
        StickyLauncherViewModel->OnWeaponViewModelUpdated.RemoveDynamic(this, &US_StickyLauncherStatusWidget::HandleViewModelUpdated);
    }

    StickyLauncherViewModel = InViewModel;

    if (StickyLauncherViewModel)
    {
        StickyLauncherViewModel->OnWeaponViewModelUpdated.AddUniqueDynamic(this, &US_StickyLauncherStatusWidget::HandleViewModelUpdated);
        RefreshWidget();
    }
    else
    {
        // When the viewmodel is cleared, hide all points
        if (Img_Point1) Img_Point1->SetVisibility(ESlateVisibility::Collapsed);
        if (Img_Point2) Img_Point2->SetVisibility(ESlateVisibility::Collapsed);
        if (Img_Point3) Img_Point3->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void US_StickyLauncherStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (StickyLauncherViewModel)
    {
        RefreshWidget();
    }
}

void US_StickyLauncherStatusWidget::NativeDestruct()
{
    if (StickyLauncherViewModel)
    {
        StickyLauncherViewModel->OnWeaponViewModelUpdated.RemoveDynamic(this, &US_StickyLauncherStatusWidget::HandleViewModelUpdated);
    }
    Super::NativeDestruct();
}

void US_StickyLauncherStatusWidget::HandleViewModelUpdated()
{
    RefreshWidget();
}

void US_StickyLauncherStatusWidget::RefreshWidget()
{
    if (!StickyLauncherViewModel)
    {
        return;
    }

    const int32 StickyCount = StickyLauncherViewModel->ActiveStickyGrenadeCount;

    // Update the visibility of each point image based on the sticky count
    if (Img_Point1)
    {
        Img_Point1->SetVisibility(StickyCount >= 1 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    if (Img_Point2)
    {
        Img_Point2->SetVisibility(StickyCount >= 2 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    if (Img_Point3)
    {
        Img_Point3->SetVisibility(StickyCount >= 3 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}