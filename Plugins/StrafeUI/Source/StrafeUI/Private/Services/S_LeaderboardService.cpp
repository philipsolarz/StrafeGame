// Plugins/StrafeUI/Source/StrafeUI/Private/Services/S_LeaderboardService.cpp

#include "Services/S_LeaderboardService.h"
#include "Engine/World.h"
#include "TimerManager.h"

void US_LeaderboardService::FetchLeaderboardData(const FString& MapName, TFunction<void(TArray<FLeaderboardEntry>)> OnComplete)
{
    if (!OnComplete)
    {
        return;
    }

    // Cancel any existing timer
    if (FetchDelayTimerHandle.IsValid())
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(FetchDelayTimerHandle);
        }
    }

    // Simulate network delay (1-2 seconds)
    float Delay = FMath::RandRange(1.0f, 2.0f);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(FetchDelayTimerHandle,
            [this, MapName, OnComplete]()
            {
                TArray<FLeaderboardEntry> Data = GenerateMockData(MapName);
                OnComplete(Data);
            },
            Delay, false);
    }
}

TArray<FString> US_LeaderboardService::GetAvailableMapNames() const
{
    TArray<FString> MapNames;
    MapNames.Add(TEXT("DM-Deck"));
    MapNames.Add(TEXT("DM-Morpheus"));
    MapNames.Add(TEXT("CTF-Face"));
    MapNames.Add(TEXT("CTF-Coret"));
    return MapNames;
}

TArray<FLeaderboardEntry> US_LeaderboardService::GenerateMockData(const FString& MapName) const
{
    TArray<FLeaderboardEntry> Entries;

    // Generate 10-15 mock entries for the requested map
    int32 NumEntries = FMath::RandRange(10, 15);

    TArray<FString> PlayerNames = {
        TEXT("FragMaster"),
        TEXT("NoobSlayer"),
        TEXT("ProGamer123"),
        TEXT("SpeedDemon"),
        TEXT("HeadshotKing"),
        TEXT("RocketJumper"),
        TEXT("SniperElite"),
        TEXT("BunnyHopper"),
        TEXT("RailgunGod"),
        TEXT("FlakMonkey"),
        TEXT("ShockRifle"),
        TEXT("InstaGibber"),
        TEXT("QuakeVet"),
        TEXT("ArenaChamp"),
        TEXT("FragHunter")
    };

    // Shuffle player names for variety
    for (int32 i = PlayerNames.Num() - 1; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        PlayerNames.Swap(i, j);
    }

    // Generate entries with progressively slower times
    float BaseTime = FMath::RandRange(45.0f, 60.0f);

    for (int32 i = 0; i < NumEntries && i < PlayerNames.Num(); i++)
    {
        FLeaderboardEntry Entry;
        Entry.PlayerName = PlayerNames[i];
        Entry.MapName = MapName;

        // Each subsequent entry is 0.5-3 seconds slower
        Entry.Time = BaseTime + (i * FMath::RandRange(0.5f, 3.0f));

        Entries.Add(Entry);
    }

    return Entries;
}