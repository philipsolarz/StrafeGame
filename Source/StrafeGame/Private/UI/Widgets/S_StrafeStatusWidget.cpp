#include "UI/Widgets/S_StrafeStatusWidget.h"
#include "UI/ViewModels/S_StrafeHUDViewModel.h"
#include "Components/TextBlock.h"

static FText FormatRaceTime(float InTime)
{
    if (InTime < 0.0f) return NSLOCTEXT("StrafeStatus", "InvalidTime", "--:--.---");
    int32 Minutes = FMath::FloorToInt(InTime / 60.f);
    int32 Seconds = FMath::FloorToInt(FMath::Fmod(InTime, 60.f));
    int32 Milliseconds = FMath::FloorToInt(FMath::Fmod(InTime * 1000.f, 1000.f));
    return FText::FromString(FString::Printf(TEXT("%02d:%02d.%03d"), Minutes, Seconds, Milliseconds));
}

static FText FormatSplitDelta(float Delta)
{
    if (FMath::IsNearlyZero(Delta)) return FText::GetEmpty();
    FString Sign = (Delta > 0.0f) ? TEXT("+") : TEXT("-");
    float AbsDelta = FMath::Abs(Delta);
    int32 Seconds = FMath::FloorToInt(AbsDelta);
    int32 Milliseconds = FMath::FloorToInt(FMath::Fmod(AbsDelta * 1000.f, 1000.f));
    return FText::FromString(FString::Printf(TEXT("(%s%d.%03d)"), *Sign, Seconds, Milliseconds));
}

void US_StrafeStatusWidget::SetViewModel(US_StrafeHUDViewModel* InViewModel)
{
    UE_LOG(LogTemp, Warning, TEXT("US_StrafeStatusWidget::SetViewModel called with VM: %s"),
        InViewModel ? *InViewModel->GetName() : TEXT("NULL"));

    if (StrafeHUDViewModel)
    {
        StrafeHUDViewModel->OnGameModeViewModelUpdated.RemoveDynamic(this, &US_StrafeStatusWidget::HandleViewModelUpdated);
    }
    StrafeHUDViewModel = InViewModel;
    if (StrafeHUDViewModel)
    {
        StrafeHUDViewModel->OnGameModeViewModelUpdated.AddUniqueDynamic(this, &US_StrafeStatusWidget::HandleViewModelUpdated);
        RefreshWidget();

        UE_LOG(LogTemp, Warning, TEXT("US_StrafeStatusWidget::SetViewModel - Bound to ViewModel updates"));
    }
}

void US_StrafeStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Warning, TEXT("US_StrafeStatusWidget::NativeConstruct - Widget: %s"), *GetName());

    if (StrafeHUDViewModel)
    {
        RefreshWidget();
    }
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
    UE_LOG(LogTemp, Warning, TEXT("US_StrafeStatusWidget::HandleViewModelUpdated - Refreshing widget"));
    RefreshWidget();
}

void US_StrafeStatusWidget::RefreshWidget()
{
    if (!StrafeHUDViewModel)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_StrafeStatusWidget::RefreshWidget - No ViewModel!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("US_StrafeStatusWidget::RefreshWidget - Time: %.2f, Checkpoint: %d/%d"),
        StrafeHUDViewModel->CurrentRaceTime,
        StrafeHUDViewModel->CurrentCheckpoint + 1,
        StrafeHUDViewModel->TotalCheckpoints);

    if (TxtCurrentTime)
    {
        TxtCurrentTime->SetText(FormatRaceTime(StrafeHUDViewModel->CurrentRaceTime));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("US_StrafeStatusWidget::RefreshWidget - TxtCurrentTime is NULL!"));
    }

    if (TxtBestTime)
    {
        TxtBestTime->SetText(FormatRaceTime(StrafeHUDViewModel->BestRaceTime.TotalTime));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("US_StrafeStatusWidget::RefreshWidget - TxtBestTime is NULL!"));
    }

    if (TxtCheckpoints)
    {
        FText CPText = FText::Format(NSLOCTEXT("StrafeStatus", "CheckpointsFmt", "CP: {0}/{1}"),
            FText::AsNumber(StrafeHUDViewModel->CurrentCheckpoint + 1),
            FText::AsNumber(StrafeHUDViewModel->TotalCheckpoints)
        );
        TxtCheckpoints->SetText(CPText);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("US_StrafeStatusWidget::RefreshWidget - TxtCheckpoints is NULL!"));
    }

    if (TxtSplits)
    {
        FString SplitsStr;
        for (int32 i = 0; i < StrafeHUDViewModel->CurrentSplitTimes.Num(); ++i)
        {
            FText SplitTimeText = FormatRaceTime(StrafeHUDViewModel->CurrentSplitTimes[i]);
            FText DeltaText = (StrafeHUDViewModel->SplitDeltas.IsValidIndex(i)) ? FormatSplitDelta(StrafeHUDViewModel->SplitDeltas[i]) : FText::GetEmpty();
            SplitsStr += FString::Printf(TEXT("CP%d: %s %s\n"), i + 1, *SplitTimeText.ToString(), *DeltaText.ToString());
        }
        TxtSplits->SetText(FText::FromString(SplitsStr));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("US_StrafeStatusWidget::RefreshWidget - TxtSplits is NULL!"));
    }
}