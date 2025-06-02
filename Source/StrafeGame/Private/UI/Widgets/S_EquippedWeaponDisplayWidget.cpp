#include "UI/Widgets/S_EquippedWeaponDisplayWidget.h"
#include "UI/ViewModels/S_PlayerHUDViewModel.h"
#include "UI/ViewModels/S_WeaponViewModel.h"
#include "UI/ViewModels/S_ChargedShotgunViewModel.h"    
#include "UI/ViewModels/S_StickyGrenadeLauncherViewModel.h" 
#include "UI/Widgets/S_ChargedShotgunStatusWidget.h"   
#include "UI/Widgets/S_StickyLauncherStatusWidget.h" 
#include "Components/WidgetSwitcher.h"
#include "Components/WidgetSwitcherSlot.h" 
#include "Blueprint/UserWidget.h" 

void US_EquippedWeaponDisplayWidget::SetPlayerHUDViewModel(US_PlayerHUDViewModel* InViewModel)
{
    if (PlayerHUDViewModel)
    {
        PlayerHUDViewModel->OnViewModelUpdated.RemoveDynamic(this, &US_EquippedWeaponDisplayWidget::HandleViewModelUpdated);
    }

    PlayerHUDViewModel = InViewModel;

    if (PlayerHUDViewModel)
    {
        PlayerHUDViewModel->OnViewModelUpdated.AddUniqueDynamic(this, &US_EquippedWeaponDisplayWidget::HandleViewModelUpdated);
        UpdateEquippedWeaponDisplay();
    }
    else
    {
        if (WeaponSpecificSwitcher)
        {
            WeaponSpecificSwitcher->SetActiveWidgetIndex(-1);
        }
    }
}

void US_EquippedWeaponDisplayWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (PlayerHUDViewModel)
    {
        UpdateEquippedWeaponDisplay();
    }
}

void US_EquippedWeaponDisplayWidget::NativeDestruct()
{
    if (PlayerHUDViewModel)
    {
        PlayerHUDViewModel->OnViewModelUpdated.RemoveDynamic(this, &US_EquippedWeaponDisplayWidget::HandleViewModelUpdated);
    }
    Super::NativeDestruct();
}

void US_EquippedWeaponDisplayWidget::HandleViewModelUpdated()
{
    UpdateEquippedWeaponDisplay();
}

UUserWidget* US_EquippedWeaponDisplayWidget::GetOrCreateWeaponSpecificWidget(TSubclassOf<UUserWidget> WidgetClassToCreate, int32& OutWidgetIndex)
{
    OutWidgetIndex = -1;
    if (!WeaponSpecificSwitcher || !WidgetClassToCreate) return nullptr;

    for (int32 i = 0; i < WeaponSpecificSwitcher->GetNumWidgets(); ++i)
    {
        if (UUserWidget* ChildWidget = Cast<UUserWidget>(WeaponSpecificSwitcher->GetWidgetAtIndex(i)))
        {
            if (ChildWidget->GetClass()->IsChildOf(WidgetClassToCreate))
            {
                OutWidgetIndex = i;
                return ChildWidget;
            }
        }
    }

    UUserWidget* NewWidget = CreateWidget(this, WidgetClassToCreate);
    if (!NewWidget)
    {
        NewWidget = CreateWidget<UUserWidget>(this, WidgetClassToCreate);
    }

    if (NewWidget)
    {
        WeaponSpecificSwitcher->AddChild(NewWidget);
        // Corrected: Use GetChildIndex from UPanelWidget (parent of UWidgetSwitcher)
        OutWidgetIndex = WeaponSpecificSwitcher->GetChildIndex(NewWidget);
        return NewWidget;
    }
    return nullptr;
}

void US_EquippedWeaponDisplayWidget::UpdateEquippedWeaponDisplay()
{
    if (!PlayerHUDViewModel || !WeaponSpecificSwitcher)
    {
        if (WeaponSpecificSwitcher) WeaponSpecificSwitcher->SetActiveWidgetIndex(-1);
        return;
    }

    US_WeaponViewModel* EquippedVM = PlayerHUDViewModel->EquippedWeaponViewModel;
    int32 ActiveWidgetIndexToShow = -1;

    if (!EquippedVM)
    {
        WeaponSpecificSwitcher->SetActiveWidgetIndex(-1);
        return;
    }

    if (US_ChargedShotgunViewModel* ChargedVM = Cast<US_ChargedShotgunViewModel>(EquippedVM))
    {
        int32 WidgetIndexInSwitcher = -1;
        US_ChargedShotgunStatusWidget* CSStatusWidget = Cast<US_ChargedShotgunStatusWidget>(
            GetOrCreateWeaponSpecificWidget(ChargedShotgunStatusWidgetClass, WidgetIndexInSwitcher)
        );

        if (CSStatusWidget)
        {
            CSStatusWidget->SetViewModel(ChargedVM);
            ActiveWidgetIndexToShow = WidgetIndexInSwitcher;
            ChargedShotgunStatusInstance = CSStatusWidget;
        }
    }
    else if (US_StickyGrenadeLauncherViewModel* StickyVM = Cast<US_StickyGrenadeLauncherViewModel>(EquippedVM))
    {
        int32 WidgetIndexInSwitcher = -1;
        US_StickyLauncherStatusWidget* SLStatusWidget = Cast<US_StickyLauncherStatusWidget>(
            GetOrCreateWeaponSpecificWidget(StickyLauncherStatusWidgetClass, WidgetIndexInSwitcher)
        );

        if (SLStatusWidget)
        {
            SLStatusWidget->SetViewModel(StickyVM);
            ActiveWidgetIndexToShow = WidgetIndexInSwitcher;
            StickyLauncherStatusInstance = SLStatusWidget;
        }
    }

    WeaponSpecificSwitcher->SetActiveWidgetIndex(ActiveWidgetIndexToShow);
}