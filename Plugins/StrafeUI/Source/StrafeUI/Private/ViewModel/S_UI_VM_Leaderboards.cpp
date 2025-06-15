// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_VM_Leaderboards.cpp

#include "ViewModel/S_UI_VM_Leaderboards.h"
#include "Engine/World.h"

void US_UI_VM_Leaderboards::Initialize()
{
    // Create the leaderboard service
    LeaderboardService = NewObject<US_LeaderboardService>(this);

    // Get available map names
    MapNames = LeaderboardService->GetAvailableMapNames();

    // Set initial map selection
    if (MapNames.Num() > 0)
    {
        CurrentMapName = MapNames[0];
        RefreshLeaderboard();
    }

    BroadcastDataChanged();
}

void US_UI_VM_Leaderboards::SetMapFilter(const FString& NewMapName)
{
    if (CurrentMapName != NewMapName)
    {
        CurrentMapName = NewMapName;
        RefreshLeaderboard();
    }
}

void US_UI_VM_Leaderboards::RefreshLeaderboard()
{
    if (!LeaderboardService || CurrentMapName.IsEmpty())
    {
        return;
    }

    // Set loading state
    bIsLoading = true;
    BroadcastDataChanged();

    // Clear existing entries
    LeaderboardEntries.Empty();

    // Fetch new data
    LeaderboardService->FetchLeaderboardData(CurrentMapName,
        [this](TArray<FLeaderboardEntry> Entries)
        {
            // Convert raw entries to view model entries
            LeaderboardEntries.Empty();

            int32 Rank = 1;
            for (const FLeaderboardEntry& Entry : Entries)
            {
                US_UI_VM_LeaderboardEntry* VMEntry = NewObject<US_UI_VM_LeaderboardEntry>(this);
                VMEntry->Rank = Rank++;
                VMEntry->PlayerName = Entry.PlayerName;
                VMEntry->MapName = Entry.MapName;
                VMEntry->Time = Entry.Time;
                VMEntry->FormattedTime = FormatTime(Entry.Time);

                LeaderboardEntries.Add(VMEntry);
            }

            // Update loading state
            bIsLoading = false;
            BroadcastDataChanged();
        });
}

void US_UI_VM_Leaderboards::PlayReplayForEntry(UObject* EntryObject)
{
    if (US_UI_VM_LeaderboardEntry* Entry = Cast<US_UI_VM_LeaderboardEntry>(EntryObject))
    {
        UE_LOG(LogTemp, Log, TEXT("Simulating replay download/play for player %s on map %s"),
            *Entry->PlayerName, *Entry->MapName);

        // In a real implementation, this would:
        // 1. Download the replay file from a server
        // 2. Save it locally
        // 3. Start playing the replay
    }
}

FString US_UI_VM_Leaderboards::FormatTime(float TimeInSeconds) const
{
    int32 Minutes = FMath::FloorToInt(TimeInSeconds / 60.0f);
    float Seconds = TimeInSeconds - (Minutes * 60.0f);
    int32 WholeSeconds = FMath::FloorToInt(Seconds);
    int32 Milliseconds = FMath::FloorToInt((Seconds - WholeSeconds) * 100.0f);

    return FString::Printf(TEXT("%02d:%02d.%02d"), Minutes, WholeSeconds, Milliseconds);
}