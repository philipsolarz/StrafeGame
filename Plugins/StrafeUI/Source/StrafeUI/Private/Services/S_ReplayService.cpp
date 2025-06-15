// Plugins/StrafeUI/Source/StrafeUI/Private/Services/S_ReplayService.cpp

#include "Services/S_ReplayService.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"

void US_ReplayService::FindLocalReplays(TFunction<void(TArray<FReplayInfo>)> OnComplete)
{
    if (!OnComplete)
    {
        return;
    }

    // Cancel any existing timer
    if (AsyncOperationTimer.IsValid())
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(AsyncOperationTimer);
        }
    }

    // Simulate async operation
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(AsyncOperationTimer,
            [this, OnComplete]()
            {
                TArray<FReplayInfo> Replays;

                FString ReplayDir = GetReplayDirectory();
                IFileManager& FileManager = IFileManager::Get();

                // Find all .replay files
                TArray<FString> ReplayFiles;
                FileManager.FindFiles(ReplayFiles, *ReplayDir, TEXT("*.replay"));

                for (const FString& FileName : ReplayFiles)
                {
                    FString FullPath = FPaths::Combine(ReplayDir, FileName);

                    FReplayInfo Info;
                    Info.FileName = FPaths::GetBaseFilename(FileName);

                    // Get file info
                    FFileStatData FileData = FileManager.GetStatData(*FullPath);
                    Info.Timestamp = FileData.ModificationTime;
                    Info.FileSizeKB = FMath::DivideAndRoundUp(FileData.FileSize, (int64)1024);

                    Replays.Add(Info);
                }

                // Sort by timestamp (newest first)
                Replays.Sort([](const FReplayInfo& A, const FReplayInfo& B)
                    {
                        return A.Timestamp > B.Timestamp;
                    });

                OnComplete(Replays);
            },
            0.1f, false);
    }
}

void US_ReplayService::PlayReplay(const FString& ReplayName, APlayerController* PC)
{
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayReplay: Invalid PlayerController"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Playing replay: %s"), *ReplayName);

    // Start playing the replay via console command
    FString Command = FString::Printf(TEXT("demoplay %s"), *ReplayName);
    PC->ConsoleCommand(Command);
}

void US_ReplayService::DeleteReplay(const FString& ReplayName, TFunction<void(bool)> OnComplete)
{
    if (!OnComplete)
    {
        return;
    }

    // Cancel any existing timer
    if (AsyncOperationTimer.IsValid())
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(AsyncOperationTimer);
        }
    }

    // Simulate async operation
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(AsyncOperationTimer,
            [this, ReplayName, OnComplete]()
            {
                FString ReplayPath = FPaths::Combine(GetReplayDirectory(), ReplayName + TEXT(".replay"));

                IFileManager& FileManager = IFileManager::Get();
                bool bSuccess = FileManager.Delete(*ReplayPath);

                if (bSuccess)
                {
                    UE_LOG(LogTemp, Log, TEXT("Successfully deleted replay: %s"), *ReplayName);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Failed to delete replay: %s"), *ReplayName);
                }

                OnComplete(bSuccess);
            },
            0.1f, false);
    }
}

FString US_ReplayService::GetReplayDirectory() const
{
    return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Demos"));
}