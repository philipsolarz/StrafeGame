// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_LeaderboardEntryWidget.cpp

#include "UI/S_UI_LeaderboardEntryWidget.h"
#include "UI/S_UI_LeaderboardsWidget.h" // <<< FIX: Added this include
#include "ViewModel/S_UI_VM_Leaderboards.h"
#include "CommonTextBlock.h"
#include "CommonButtonBase.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"

void US_UI_LeaderboardEntryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Btn_PlayReplay)
    {
        Btn_PlayReplay->OnClicked().AddUObject(this, &US_UI_LeaderboardEntryWidget::OnPlayReplayClicked);
    }
}

void US_UI_LeaderboardEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (US_UI_VM_LeaderboardEntry* Entry = Cast<US_UI_VM_LeaderboardEntry>(ListItemObject))
    {
        CachedEntryData = Entry;

        // Update rank display
        if (Text_Rank)
        {
            Text_Rank->SetText(FText::AsNumber(Entry->Rank));

            // Special color for top 3
            FLinearColor RankColor = GetRankColor(Entry->Rank);
            Text_Rank->SetColorAndOpacity(FSlateColor(RankColor));
        }

        // Update player name
        if (Text_PlayerName)
        {
            Text_PlayerName->SetText(FText::FromString(Entry->PlayerName));
        }

        // Update time
        if (Text_Time)
        {
            Text_Time->SetText(FText::FromString(Entry->FormattedTime));
        }

        // Update rank icon if present (for medals)
        if (Img_RankIcon)
        {
            // Show medal icon for top 3
            if (Entry->Rank <= 3)
            {
                Img_RankIcon->SetVisibility(ESlateVisibility::HitTestInvisible);

                // Set color based on rank
                FLinearColor MedalColor = GetRankColor(Entry->Rank);
                Img_RankIcon->SetColorAndOpacity(MedalColor);
            }
            else
            {
                Img_RankIcon->SetVisibility(ESlateVisibility::Collapsed);
            }
        }

        // Find the parent view model through the widget tree
        if (UWidget* ParentWidget = GetParent())
        {
            while (ParentWidget)
            {
                if (US_UI_LeaderboardsWidget* LeaderboardWidget = Cast<US_UI_LeaderboardsWidget>(ParentWidget))
                {
                    // Access the view model through the CreateViewModel function
                    if (US_UI_ViewModelBase* BaseVM = LeaderboardWidget->CreateViewModel())
                    {
                        ParentViewModel = Cast<US_UI_VM_Leaderboards>(BaseVM);
                    }
                    break;
                }
                ParentWidget = ParentWidget->GetParent();
            }
        }
    }
}

void US_UI_LeaderboardEntryWidget::OnPlayReplayClicked()
{
    if (CachedEntryData.IsValid() && ParentViewModel.IsValid())
    {
        ParentViewModel->PlayReplayForEntry(CachedEntryData.Get());
    }
}

FLinearColor US_UI_LeaderboardEntryWidget::GetRankColor(int32 Rank) const
{
    switch (Rank)
    {
    case 1:
        // Gold
        return FLinearColor(1.0f, 0.843f, 0.0f, 1.0f);
    case 2:
        // Silver
        return FLinearColor(0.753f, 0.753f, 0.753f, 1.0f);
    case 3:
        // Bronze
        return FLinearColor(0.804f, 0.498f, 0.196f, 1.0f);
    default:
        // Normal
        return FLinearColor::White;
    }
}