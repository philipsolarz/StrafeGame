#include "UI/Widgets/S_ChargedShotgunStatusWidget.h"
#include "UI/ViewModels/S_ChargedShotgunViewModel.h"
#include "Components/Image.h" // Include UImage for GetDynamicMaterial
#include "Materials/MaterialInstanceDynamic.h" // Include for UMaterialInstanceDynamic

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
        // When the viewmodel is cleared, hide the images
        if (Img_PrimaryCharge) Img_PrimaryCharge->SetVisibility(ESlateVisibility::Collapsed);
        if (Img_SecondaryCharge) Img_SecondaryCharge->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void US_ChargedShotgunStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Create Dynamic Material Instances once the widget is constructed.
    // This ensures we have a unique material instance to modify without affecting the base material.
    if (Img_PrimaryCharge && !PrimaryChargeMID)
    {
        PrimaryChargeMID = Img_PrimaryCharge->GetDynamicMaterial();
        Img_PrimaryCharge->SetVisibility(ESlateVisibility::Visible);
    }
    if (Img_SecondaryCharge && !SecondaryChargeMID)
    {
        SecondaryChargeMID = Img_SecondaryCharge->GetDynamicMaterial();
        Img_SecondaryCharge->SetVisibility(ESlateVisibility::Visible);
    }

    if (ChargedShotgunViewModel)
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

    // Update the scalar parameter on our dynamic material instances
    if (PrimaryChargeMID)
    {
        PrimaryChargeMID->SetScalarParameterValue(FName("CircularProgress006"), ChargedShotgunViewModel->PrimaryChargeProgress);
    }
    if (SecondaryChargeMID)
    {
        SecondaryChargeMID->SetScalarParameterValue(FName("CircularProgress006"), ChargedShotgunViewModel->SecondaryChargeProgress);
    }
}