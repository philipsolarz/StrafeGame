#include "UI/Widgets/S_PlayerStatusWidget.h"
#include "UI/ViewModels/S_PlayerHUDViewModel.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void US_PlayerStatusWidget::SetViewModel(US_PlayerHUDViewModel* InViewModel)
{
    if (PlayerHUDViewModel)
    {
        PlayerHUDViewModel->OnViewModelUpdated.RemoveDynamic(this, &US_PlayerStatusWidget::HandleViewModelUpdated);
    }

    PlayerHUDViewModel = InViewModel;

    if (PlayerHUDViewModel)
    {
        PlayerHUDViewModel->OnViewModelUpdated.AddUniqueDynamic(this, &US_PlayerStatusWidget::HandleViewModelUpdated);
        RefreshWidget();
    }
    else
    {
        // Clear display if ViewModel is null
        if (TxtHealth) TxtHealth->SetText(FText::GetEmpty());
        if (PBHealth) PBHealth->SetPercent(0.f);
        if (TxtArmor) TxtArmor->SetText(FText::GetEmpty());
        if (PBArmor) PBArmor->SetPercent(0.f);
    }
}

void US_PlayerStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // ViewModel should be set by the parent (e.g., WBP_MainHUD) via SetViewModel
    // If PlayerHUDViewModel is already valid here (e.g. set in BP defaults for testing), refresh
    if (PlayerHUDViewModel)
    {
        RefreshWidget();
    }
}

void US_PlayerStatusWidget::NativeDestruct()
{
    if (PlayerHUDViewModel)
    {
        PlayerHUDViewModel->OnViewModelUpdated.RemoveDynamic(this, &US_PlayerStatusWidget::HandleViewModelUpdated);
    }
    Super::NativeDestruct();
}

void US_PlayerStatusWidget::HandleViewModelUpdated()
{
    RefreshWidget();
}

void US_PlayerStatusWidget::RefreshWidget()
{
    if (!PlayerHUDViewModel)
    {
        return;
    }

    if (TxtHealth)
    {
        TxtHealth->SetText(FText::AsNumber(FMath::CeilToInt(PlayerHUDViewModel->CurrentHealth)));
    }
    if (PBHealth)
    {
        PBHealth->SetPercent(PlayerHUDViewModel->HealthPercentage);
    }

    if (TxtArmor)
    {
        TxtArmor->SetText(FText::AsNumber(FMath::CeilToInt(PlayerHUDViewModel->CurrentArmor)));
    }
    if (PBArmor)
    {
        PBArmor->SetPercent(PlayerHUDViewModel->ArmorPercentage);
    }
}