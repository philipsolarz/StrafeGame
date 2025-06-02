#include "UI/ViewModels/S_AnnouncerViewModel.h"
#include "TimerManager.h"
#include "Engine/World.h"

void US_AnnouncerViewModel::Initialize(AS_PlayerController* InOwningPlayerController)
{
    Super::Initialize(InOwningPlayerController);
    bIsAnnouncementVisible = false;
    CurrentAnnouncementText = FText::GetEmpty();
}

void US_AnnouncerViewModel::Deinitialize()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AnnouncementTimerHandle);
    }
    Super::Deinitialize();
}

void US_AnnouncerViewModel::ShowAnnouncement(const FText& Message, float Duration)
{
    CurrentAnnouncementText = Message;
    bIsAnnouncementVisible = true;
    OnAnnouncementChanged.Broadcast();

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(AnnouncementTimerHandle, this, &US_AnnouncerViewModel::HideAnnouncement, Duration, false);
    }
}

void US_AnnouncerViewModel::HideAnnouncement()
{
    bIsAnnouncementVisible = false;
    CurrentAnnouncementText = FText::GetEmpty(); // Clear after hiding
    OnAnnouncementChanged.Broadcast();
}