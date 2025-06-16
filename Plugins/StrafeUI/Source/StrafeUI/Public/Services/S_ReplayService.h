// Plugins/StrafeUI/Source/StrafeUI/Public/Services/S_ReplayService.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "S_ReplayService.generated.h"

/**
 * Struct representing a replay file
 */
USTRUCT(BlueprintType)
struct FReplayInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString FileName;

    UPROPERTY(BlueprintReadOnly)
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadOnly)
    int32 FileSizeKB;

    FReplayInfo()
    {
        FileSizeKB = 0;
        Timestamp = FDateTime::Now();
    }
};

/**
 * Service for managing local replay files
 */
UCLASS()
class STRAFEUI_API US_ReplayService : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Finds all local replay files
     * @param OnComplete Callback function that receives the replay information
     */
    void FindLocalReplays(TFunction<void(TArray<FReplayInfo>)> OnComplete);

    /**
     * Plays a replay
     * @param ReplayName The name of the replay to play (without extension)
     * @param PC The player controller to use for playing the replay
     */
    UFUNCTION(BlueprintCallable, Category = "Replay")
    void PlayReplay(const FString& ReplayName, APlayerController* PC);

    /**
     * Deletes a replay file
     * @param ReplayName The name of the replay to delete (without extension)
     * @param OnComplete Callback function that receives success/failure
     */
    void DeleteReplay(const FString& ReplayName, TFunction<void(bool)> OnComplete);

private:
    /** Gets the replay directory path */
    FString GetReplayDirectory() const;

    /** Timer handle for async operations */
    FTimerHandle AsyncOperationTimer;
};