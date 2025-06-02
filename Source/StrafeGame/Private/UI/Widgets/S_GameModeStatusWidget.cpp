#include "UI/Widgets/S_GameModeStatusWidget.h"
#include "UI/ViewModels/S_GameModeHUDViewModelBase.h"
#include "UI/ViewModels/S_ArenaHUDViewModel.h"   // For casting
#include "UI/ViewModels/S_StrafeHUDViewModel.h"  // For casting
#include "UI/Widgets/S_ArenaStatusWidget.h"      // For creation/casting
#include "UI/Widgets/S_StrafeStatusWidget.h"     // For creation/casting
#include "Components/WidgetSwitcher.h"
#include "Blueprint/UserWidget.h" // For CreateWidget

void US_GameModeStatusWidget::SetViewModel(US_GameModeHUDViewModelBase* InViewModel)
{
    if (GameModeViewModel)
    {
        GameModeViewModel->OnGameModeViewModelUpdated.RemoveDynamic(this, &US_GameModeStatusWidget::HandleViewModelUpdated);
    }

    GameModeViewModel = InViewModel;

    if (GameModeViewModel)
    {
        GameModeViewModel->OnGameModeViewModelUpdated.AddUniqueDynamic(this, &US_GameModeStatusWidget::HandleViewModelUpdated);
        UpdateActiveGameModeWidget();
    }
    else
    {
        if (GameModeSpecificSwitcher) GameModeSpecificSwitcher->SetActiveWidgetIndex(-1);
    }
}

void US_GameModeStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (GameModeViewModel) // If set via BP for testing or by parent immediately
    {
        UpdateActiveGameModeWidget();
    }
}

void US_GameModeStatusWidget::NativeDestruct()
{
    if (GameModeViewModel)
    {
        GameModeViewModel->OnGameModeViewModelUpdated.RemoveDynamic(this, &US_GameModeStatusWidget::HandleViewModelUpdated);
    }
    Super::NativeDestruct();
}

void US_GameModeStatusWidget::HandleViewModelUpdated()
{
    UpdateActiveGameModeWidget();
}

UUserWidget* US_GameModeStatusWidget::GetOrCreateGameModeSpecificWidget(TSubclassOf<UUserWidget> WidgetClassToCreate, int32& OutWidgetIndexInSwitcher)
{
    OutWidgetIndexInSwitcher = -1;
    if (!GameModeSpecificSwitcher || !WidgetClassToCreate) return nullptr;

    for (int32 i = 0; i < GameModeSpecificSwitcher->GetNumWidgets(); ++i)
    {
        if (UUserWidget* ChildWidget = Cast<UUserWidget>(GameModeSpecificSwitcher->GetWidgetAtIndex(i)))
        {
            if (ChildWidget->GetClass()->IsChildOf(WidgetClassToCreate))
            {
                OutWidgetIndexInSwitcher = i;
                return ChildWidget;
            }
        }
    }

    UUserWidget* NewWidget = CreateWidget(this, WidgetClassToCreate);
    if (NewWidget)
    {
        GameModeSpecificSwitcher->AddChild(NewWidget);
        OutWidgetIndexInSwitcher = GameModeSpecificSwitcher->GetChildIndex(NewWidget);
        return NewWidget;
    }
    return nullptr;
}


void US_GameModeStatusWidget::UpdateActiveGameModeWidget()
{
    if (!GameModeViewModel || !GameModeSpecificSwitcher)
    {
        if (GameModeSpecificSwitcher) GameModeSpecificSwitcher->SetActiveWidgetIndex(-1);
        return;
    }

    int32 ActiveWidgetIndexToShow = -1;

    if (US_ArenaHUDViewModel* ArenaVM = Cast<US_ArenaHUDViewModel>(GameModeViewModel))
    {
        int32 WidgetIndex = -1;
        US_ArenaStatusWidget* ArenaWidget = Cast<US_ArenaStatusWidget>(
            GetOrCreateGameModeSpecificWidget(ArenaStatusWidgetClass, WidgetIndex)
        );
        if (ArenaWidget)
        {
            ArenaWidget->SetViewModel(ArenaVM); // ArenaStatusWidget needs SetViewModel(US_ArenaHUDViewModel*)
            ActiveWidgetIndexToShow = WidgetIndex;
            ArenaStatusWidgetInstance = ArenaWidget;
        }
    }
    else if (US_StrafeHUDViewModel* StrafeVM = Cast<US_StrafeHUDViewModel>(GameModeViewModel))
    {
        int32 WidgetIndex = -1;
        US_StrafeStatusWidget* StrafeWidget = Cast<US_StrafeStatusWidget>(
            GetOrCreateGameModeSpecificWidget(StrafeStatusWidgetClass, WidgetIndex)
        );
        if (StrafeWidget)
        {
            StrafeWidget->SetViewModel(StrafeVM); // StrafeStatusWidget needs SetViewModel(US_StrafeHUDViewModel*)
            ActiveWidgetIndexToShow = WidgetIndex;
            StrafeStatusWidgetInstance = StrafeWidget;
        }
    }
    // Add more game modes here

    GameModeSpecificSwitcher->SetActiveWidgetIndex(ActiveWidgetIndexToShow);
}