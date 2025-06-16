// Plugins/StrafeUI/Source/StrafeUI/Public/Services/S_LeaderboardService.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "S_LeaderboardService.generated.h"

/**
 * Struct representing a single leaderboard entry
 */
USTRUCT(BlueprintType)
struct FLeaderboardEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString PlayerName;

    UPROPERTY(BlueprintReadOnly)
    FString MapName;

    UPROPERTY(BlueprintReadOnly)
    float Time;

    FLeaderboardEntry()
    {
        Time = 0.0f;
    }
};

/**
 * Mock service for fetching leaderboard data
 * In a real implementation, this would communicate with a backend service
 */
UCLASS()
class STRAFEUI_API US_LeaderboardService : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Fetches leaderboard data for a specific map
     * @param MapName The name of the map to fetch leaderboard data for
     * @param OnComplete Callback function that receives the leaderboard entries
     */
    void FetchLeaderboardData(const FString& MapName, TFunction<void(TArray<FLeaderboardEntry>)> OnComplete);

    /**
     * Gets a list of all available map names that have leaderboards
     * @return Array of map names
     */
    TArray<FString> GetAvailableMapNames() const;

private:
    /** Timer handle for simulating network delay */
    FTimerHandle FetchDelayTimerHandle;

    /** Generates mock leaderboard data */
    TArray<FLeaderboardEntry> GenerateMockData(const FString& MapName) const;
};