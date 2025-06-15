// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_ReplayListEntryWidget.cpp

#include "UI/S_UI_ReplayListEntryWidget.h"
#include "ViewModel/S_UI_VM_Replays.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"

void US_UI_ReplayListEntryWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    // Set default icon color if present
    if (Img_ReplayIcon)
    {
        Img_ReplayIcon->SetColorAndOpacity(FLinearColor::White);
    }
}

void US_UI_ReplayListEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (US_UI_VM_ReplayEntry* Entry = Cast<US_UI_VM_ReplayEntry>(ListItemObject))
    {
        CachedReplayData = Entry;

        // Update file name
        if (Text_FileName)
        {
            Text_FileName->SetText(FText::FromString(Entry->FileName));
        }

        // Split timestamp into date and time
        if (Text_Date && Text_Time)
        {
            FString DateStr = Entry->Timestamp.ToString(TEXT("%Y-%m-%d"));
            FString TimeStr = Entry->Timestamp.ToString(TEXT("%H:%M"));

            Text_Date->SetText(FText::FromString(DateStr));
            Text_Time->SetText(FText::FromString(TimeStr));
        }

        // Update file size
        if (Text_FileSize)
        {
            Text_FileSize->SetText(FText::FromString(Entry->FileSizeText));
        }

        // Update icon color based on age
        if (Img_ReplayIcon)
        {
            FLinearColor AgeColor = GetAgeColor(Entry->Timestamp);
            Img_ReplayIcon->SetColorAndOpacity(AgeColor);
        }
    }
}

FLinearColor US_UI_ReplayListEntryWidget::GetAgeColor(const FDateTime& Timestamp) const
{
    FDateTime Now = FDateTime::Now();
    FTimespan Age = Now - Timestamp;

    int32 DaysOld = Age.GetDays();

    if (DaysOld < 1)
    {
        // Less than 1 day old - Bright green
        return FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);
    }
    else if (DaysOld < 7)
    {
        // Less than 1 week old - Green
        return FLinearColor(0.5f, 1.0f, 0.5f, 1.0f);
    }
    else if (DaysOld < 30)
    {
        // Less than 1 month old - Yellow
        return FLinearColor(1.0f, 1.0f, 0.3f, 1.0f);
    }
    else if (DaysOld < 90)
    {
        // Less than 3 months old - Orange
        return FLinearColor(1.0f, 0.7f, 0.3f, 1.0f);
    }
    else
    {
        // Older than 3 months - Gray
        return FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);
    }
}