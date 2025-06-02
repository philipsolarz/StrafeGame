#include "UI/Widgets/S_AnnouncementsDisplayWidget.h"
#include "UI/ViewModels/S_AnnouncerViewModel.h"
#include "Components/TextBlock.h"

void US_AnnouncementsDisplayWidget::SetViewModel(US_AnnouncerViewModel* InViewModel)
{
    if (AnnouncerViewModel)
    {
        AnnouncerViewModel->OnAnnouncementChanged.RemoveDynamic(this, &US_AnnouncementsDisplayWidget::HandleViewModelUpdated);
    }
    AnnouncerViewModel = InViewModel;
    if (AnnouncerViewModel)
    {
        AnnouncerViewModel->OnAnnouncementChanged.AddUniqueDynamic(this, &US_AnnouncementsDisplayWidget::HandleViewModelUpdated);
        RefreshWidget();
    }
    else
    {
        if (TxtAnnouncement) TxtAnnouncement->SetText(FText::GetEmpty());
        SetVisibility(ESlateVisibility::Collapsed);
    }
}

void US_AnnouncementsDisplayWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (AnnouncerViewModel) RefreshWidget();
}

void US_AnnouncementsDisplayWidget::NativeDestruct()
{
    if (AnnouncerViewModel)
    {
        AnnouncerViewModel->OnAnnouncementChanged.RemoveDynamic(this, &US_AnnouncementsDisplayWidget::HandleViewModelUpdated);
    }
    Super::NativeDestruct();
}

void US_AnnouncementsDisplayWidget::HandleViewModelUpdated()
{
    RefreshWidget();
}

void US_AnnouncementsDisplayWidget::RefreshWidget()
{
    if (!AnnouncerViewModel) return;

    if (TxtAnnouncement)
    {
        TxtAnnouncement->SetText(AnnouncerViewModel->CurrentAnnouncementText);
    }
    // Control overall visibility of this widget or an animation based on AnnouncerViewModel->bIsAnnouncementVisible
    SetVisibility(AnnouncerViewModel->bIsAnnouncementVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

    // If you have animations for fade in/out in your WBP_AnnouncementsDisplay,
    // you can play them here based on bIsAnnouncementVisible.
    // Example:
    // if (AnnouncerViewModel->bIsAnnouncementVisible && FadeInAnimation)
    // {
    //     PlayAnimation(FadeInAnimation);
    // }
    // else if (!AnnouncerViewModel->bIsAnnouncementVisible && FadeOutAnimation)
    // {
    //     PlayAnimation(FadeOutAnimation);
    // }
}