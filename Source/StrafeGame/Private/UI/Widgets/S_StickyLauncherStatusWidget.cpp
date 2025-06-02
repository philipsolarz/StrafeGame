#include "UI/Widgets/S_StickyLauncherStatusWidget.h"
#include "UI/ViewModels/S_StickyGrenadeLauncherViewModel.h"
#include "Components/TextBlock.h"

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
        if (TxtActiveStickies) TxtActiveStickies->SetText(FText::GetEmpty());
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

    if (TxtActiveStickies)
    {
        TxtActiveStickies->SetText(FText::AsNumber(StickyLauncherViewModel->ActiveStickyGrenadeCount));
    }
}