// Source/StrafeGame/Private/UI/Widgets/S_ArenaStatusWidget.cpp
#include "UI/Widgets/S_ArenaStatusWidget.h"
#include "UI/ViewModels/S_ArenaHUDViewModel.h"
#include "UI/ViewModels/S_KillfeedItemViewModel.h" 
#include "Components/TextBlock.h"
#include "Components/ListView.h"

void US_ArenaStatusWidget::SetViewModel(US_ArenaHUDViewModel* InViewModel)
{
    if (ArenaHUDViewModel)
    {
        ArenaHUDViewModel->OnGameModeViewModelUpdated.RemoveDynamic(this, &US_ArenaStatusWidget::HandleViewModelUpdated);
    }

    ArenaHUDViewModel = InViewModel;

    if (ArenaHUDViewModel)
    {
        ArenaHUDViewModel->OnGameModeViewModelUpdated.AddUniqueDynamic(this, &US_ArenaStatusWidget::HandleViewModelUpdated);
        RefreshWidget();
    }
    else
    {
        if (TxtTimeLimit) TxtTimeLimit->SetText(FText::GetEmpty());
        if (TxtFragLimit) TxtFragLimit->SetText(FText::GetEmpty());
        if (TxtPlayerScore) TxtPlayerScore->SetText(FText::GetEmpty());
        if (TxtLeaderScore) TxtLeaderScore->SetText(FText::GetEmpty());
        if (KillfeedListView) KillfeedListView->ClearListItems();
    }
}

void US_ArenaStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (ArenaHUDViewModel)
    {
        RefreshWidget();
    }
}

void US_ArenaStatusWidget::NativeDestruct()
{
    if (ArenaHUDViewModel)
    {
        ArenaHUDViewModel->OnGameModeViewModelUpdated.RemoveDynamic(this, &US_ArenaStatusWidget::HandleViewModelUpdated);
    }
    Super::NativeDestruct();
}

void US_ArenaStatusWidget::HandleViewModelUpdated()
{
    RefreshWidget();
}

void US_ArenaStatusWidget::RefreshWidget()
{
    if (!ArenaHUDViewModel) return;

    if (TxtTimeLimit)
    {
        TxtTimeLimit->SetText(FText::Format(NSLOCTEXT("ArenaStatus", "TimeLimitFmt", "Time: {0}"), ArenaHUDViewModel->RemainingMatchTimeSeconds));
    }
    if (TxtFragLimit)
    {
        TxtFragLimit->SetText(FText::Format(NSLOCTEXT("ArenaStatus", "FragLimitFmt", "Frag Limit: {0}"), ArenaHUDViewModel->FragLimit));
    }
    if (TxtPlayerScore)
    {
        TxtPlayerScore->SetText(FText::Format(NSLOCTEXT("ArenaStatus", "PlayerScoreFmt", "Score: {0} / Deaths: {1}"), ArenaHUDViewModel->PlayerFrags, ArenaHUDViewModel->PlayerDeaths));
    }
    if (TxtLeaderScore)
    {
        TxtLeaderScore->SetText(FText::Format(NSLOCTEXT("ArenaStatus", "LeaderScoreFmt", "Leader: {0} ({1})"), FText::FromString(ArenaHUDViewModel->LeaderName), ArenaHUDViewModel->LeaderFrags));
    }

    RefreshKillfeed();
}

void US_ArenaStatusWidget::RefreshKillfeed()
{
    if (!ArenaHUDViewModel || !KillfeedListView) return;

    KillfeedListView->ClearListItems();
    // Correctly iterates over the array of US_KillfeedItemViewModel pointers
    for (US_KillfeedItemViewModel* KillVM : ArenaHUDViewModel->KillfeedItemViewModels)
    {
        if (KillVM)
        {
            KillfeedListView->AddItem(KillVM); // Add the UObject pointer
        }
    }
}