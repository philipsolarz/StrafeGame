#include "UI/Widgets/S_StrafeStatusWidget.h"
#include "UI/ViewModels/S_StrafeHUDViewModel.h"
#include "Components/TextBlock.h"

// Helper to format time (float seconds to MM:SS.mmm)
static FText FormatRaceTime(float InTime)
{
    if (InTime < 0.0f) return NSLOCTEXT("StrafeStatus", "InvalidTime", "--:--.---");
    int32 Minutes = FMath::FloorToInt(InTime / 60.f);
    int32 Seconds = FMath::FloorToInt(FMath::Fmod(InTime, 60.f));
    int32 Milliseconds = FMath::FloorToInt(FMath::Fmod(InTime * 1000.f, 1000.f));
    return FText::FromString(FString::Printf(TEXT("%02d:%02d.%03d"), Minutes, Seconds, Milliseconds));
}

void US_StrafeStatusWidget::SetViewModel(US_StrafeHUDViewModel* InViewModel)
{
    if (StrafeHUDViewModel)
    {
        StrafeHUDViewModel->OnGameModeViewModelUpdated.RemoveDynamic(this, &US_StrafeStatusWidget::HandleViewModelUpdated);
    }
    StrafeHUDViewModel = InViewModel;
    if (StrafeHUDViewModel)
    {
        StrafeHUDViewModel->OnGameModeViewModelUpdated.AddUniqueDynamic(this, &US_StrafeStatusWidget::HandleViewModelUpdated);
        RefreshWidget();
    }
    else
    {
        // Clear fields
        if (TxtCurrentTime) TxtCurrentTime->SetText(FText::GetEmpty());
        if (TxtBestTime) TxtBestTime->SetText(FText::GetEmpty());
        if (TxtSplits) TxtSplits->SetText(FText::GetEmpty());
        if (TxtCheckpoints) TxtCheckpoints->SetText(FText::GetEmpty());
    }
}

void US_StrafeStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (StrafeHUDViewModel) RefreshWidget();
}

void US_StrafeStatusWidget::NativeDestruct()
{
    if (StrafeHUDViewModel)
    {
        StrafeHUDViewModel->OnGameModeViewModelUpdated.RemoveDynamic(this, &US_StrafeStatusWidget::HandleViewModelUpdated);
    }
    Super::NativeDestruct();
}

void US_StrafeStatusWidget::HandleViewModelUpdated()
{
    RefreshWidget();
}

void US_StrafeStatusWidget::RefreshWidget()
{
    if (!StrafeHUDViewModel) return;

    if (TxtCurrentTime)
    {
        TxtCurrentTime->SetText(FormatRaceTime(StrafeHUDViewModel->CurrentRaceTime));
    }
    if (TxtBestTime)
    {
        TxtBestTime->SetText(FormatRaceTime(StrafeHUDViewModel->BestRaceTime.TotalTime));
    }
    if (TxtCheckpoints)
    {
        FText CPText = FText::Format(NSLOCTEXT("StrafeStatus", "CheckpointsFmt", "CP: {0}/{1}"),
            FText::AsNumber(StrafeHUDViewModel->CurrentCheckpoint + 1), // +1 for display if 0-indexed
            FText::AsNumber(StrafeHUDViewModel->TotalCheckpoints)
        );
        TxtCheckpoints->SetText(CPText);
    }
    if (TxtSplits) // Basic split display, could be a ListView for more detail
    {
        FString SplitsStr;
        for (int32 i = 0; i < StrafeHUDViewModel->CurrentSplitTimes.Num(); ++i)
        {
            SplitsStr += FString::Printf(TEXT("CP%d: %s\n"), i + 1, *FormatRaceTime(StrafeHUDViewModel->CurrentSplitTimes[i]).ToString());
        }
        TxtSplits->SetText(FText::FromString(SplitsStr));
    }
}