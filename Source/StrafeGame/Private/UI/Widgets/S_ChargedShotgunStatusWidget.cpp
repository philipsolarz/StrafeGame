#include "UI/Widgets/S_ChargedShotgunStatusWidget.h"
#include "UI/ViewModels/S_ChargedShotgunViewModel.h"
#include "Components/ProgressBar.h"

void US_ChargedShotgunStatusWidget::SetViewModel(US_ChargedShotgunViewModel* InViewModel)
{
    if (ChargedShotgunViewModel)
    {
        // OnWeaponViewModelUpdated is from the base US_WeaponViewModel
        ChargedShotgunViewModel->OnWeaponViewModelUpdated.RemoveDynamic(this, &US_ChargedShotgunStatusWidget::HandleViewModelUpdated);
    }

    ChargedShotgunViewModel = InViewModel;

    if (ChargedShotgunViewModel)
    {
        ChargedShotgunViewModel->OnWeaponViewModelUpdated.AddUniqueDynamic(this, &US_ChargedShotgunStatusWidget::HandleViewModelUpdated);
        RefreshWidget();
    }
    else
    {
        if (PBPrimaryCharge) PBPrimaryCharge->SetPercent(0.f);
        if (PBSecondaryCharge) PBSecondaryCharge->SetPercent(0.f);
    }
}

void US_ChargedShotgunStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (ChargedShotgunViewModel) // If set in BP for testing or by parent immediately
    {
        RefreshWidget();
    }
}

void US_ChargedShotgunStatusWidget::NativeDestruct()
{
    if (ChargedShotgunViewModel)
    {
        ChargedShotgunViewModel->OnWeaponViewModelUpdated.RemoveDynamic(this, &US_ChargedShotgunStatusWidget::HandleViewModelUpdated);
    }
    Super::NativeDestruct();
}

void US_ChargedShotgunStatusWidget::HandleViewModelUpdated()
{
    RefreshWidget();
}

void US_ChargedShotgunStatusWidget::RefreshWidget()
{
    if (!ChargedShotgunViewModel)
    {
        return;
    }

    if (PBPrimaryCharge)
    {
        PBPrimaryCharge->SetPercent(ChargedShotgunViewModel->PrimaryChargeProgress);
    }
    if (PBSecondaryCharge)
    {
        PBSecondaryCharge->SetPercent(ChargedShotgunViewModel->SecondaryChargeProgress);
    }
}