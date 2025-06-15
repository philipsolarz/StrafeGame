// Plugins/StrafeUI/Source/StrafeUI/Public/ViewModel/S_UI_VM_Leaderboards.h

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/S_UI_ViewModelBase.h"
#include "Services/S_LeaderboardService.h"
#include "S_UI_VM_Leaderboards.generated.h"

/**
 * ViewModel entry for leaderboard list items
 */
UCLASS()
class STRAFEUI_API US_UI_VM_LeaderboardEntry : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly)
    int32 Rank;

    UPROPERTY(BlueprintReadOnly)
    FString PlayerName;

    UPROPERTY(BlueprintReadOnly)
    FString MapName;

    UPROPERTY(BlueprintReadOnly)
    float Time;

    UPROPERTY(BlueprintReadOnly)
    FString FormattedTime;
};

/**
 * ViewModel for the Leaderboards screen
 */
UCLASS(BlueprintType)
class STRAFEUI_API US_UI_VM_Leaderboards : public US_UI_ViewModelBase
{
    GENERATED_BODY()

public:
    /** Initializes the ViewModel */
    UFUNCTION(BlueprintCallable, Category = "Leaderboards")
    void Initialize();

    /** Sets the map filter and refreshes the leaderboard */
    UFUNCTION(BlueprintCallable, Category = "Leaderboards")
    void SetMapFilter(const FString& NewMapName);

    /** Refreshes the leaderboard data */
    UFUNCTION(BlueprintCallable, Category = "Leaderboards")
    void RefreshLeaderboard();

    /** Placeholder function to simulate playing a replay */
    UFUNCTION(BlueprintCallable, Category = "Leaderboards")
    void PlayReplayForEntry(UObject* EntryObject);

    /** List of leaderboard entries */
    UPROPERTY(BlueprintReadOnly, Category = "Leaderboards")
    TArray<UObject*> LeaderboardEntries;

    /** List of available map names */
    UPROPERTY(BlueprintReadOnly, Category = "Leaderboards")
    TArray<FString> MapNames;

    /** Currently selected map filter */
    UPROPERTY(BlueprintReadOnly, Category = "Leaderboards")
    FString CurrentMapName;

    /** Whether data is currently being loaded */
    UPROPERTY(BlueprintReadOnly, Category = "Leaderboards")
    bool bIsLoading;

private:
    /** Cached leaderboard service */
    UPROPERTY()
    TObjectPtr<US_LeaderboardService> LeaderboardService;

    /** Formats time in seconds to MM:SS.MS format */
    FString FormatTime(float TimeInSeconds) const;
};