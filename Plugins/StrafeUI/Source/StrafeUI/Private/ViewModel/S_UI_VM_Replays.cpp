// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_VM_Replays.cpp

#include "ViewModel/S_UI_VM_Replays.h"
#include "S_UI_Subsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

void US_UI_VM_Replays::Initialize()
{
    // Create the replay service
    ReplayService = NewObject<US_ReplayService>(this);

    // Load initial replay list
    RefreshReplays();
}

void US_UI_VM_Replays::RefreshReplays()
{
    if (!ReplayService)
    {
        return;
    }

    // Set loading state
    bIsLoading = true;
    BroadcastDataChanged();

    // Clear existing entries
    ReplayEntries.Empty();
    SelectedReplay = nullptr;

    // Find replays
    ReplayService->FindLocalReplays(
        [this](TArray<FReplayInfo> Replays)
        {
            // Convert raw entries to view model entries
            ReplayEntries.Empty();

            for (const FReplayInfo& Info : Replays)
            {
                US_UI_VM_ReplayEntry* Entry = NewObject<US_UI_VM_ReplayEntry>(this);
                Entry->FileName = Info.FileName;
                Entry->Timestamp = Info.Timestamp;
                Entry->FileSizeKB = Info.FileSizeKB;
                Entry->FormattedTimestamp = FormatTimestamp(Info.Timestamp);
                Entry->FileSizeText = FormatFileSize(Info.FileSizeKB);

                ReplayEntries.Add(Entry);
            }

            // Update loading state
            bIsLoading = false;
            BroadcastDataChanged();
        });
}

void US_UI_VM_Replays::PlaySelectedReplay(APlayerController* PC)
{
    if (!ReplayService || !SelectedReplay || !PC)
    {
        return;
    }

    if (US_UI_VM_ReplayEntry* Entry = Cast<US_UI_VM_ReplayEntry>(SelectedReplay))
    {
        ReplayService->PlayReplay(Entry->FileName, PC);
    }
}

void US_UI_VM_Replays::DeleteSelectedReplay()
{
    if (!SelectedReplay)
    {
        return;
    }

    // Show confirmation modal
    if (UWorld* World = GetWorld())
    {
        if (US_UI_Subsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
        {
            F_UIModalPayload Payload;
            Payload.Message = FText::FromString(TEXT("Are you sure you want to delete this replay? This action cannot be undone."));
            Payload.ModalType = E_UIModalType::YesNo;

            UISubsystem->RequestModal(Payload, FOnModalDismissedSignature::CreateUObject(
                this, &US_UI_VM_Replays::PerformDeleteReplay));
        }
    }
}

void US_UI_VM_Replays::SetSelectedReplay(UObject* ReplayEntry)
{
    if (SelectedReplay != ReplayEntry)
    {
        SelectedReplay = ReplayEntry;
        BroadcastDataChanged();
    }
}

// <<< FIX: Added bConfirmed parameter and logic to handle it
void US_UI_VM_Replays::PerformDeleteReplay(bool bConfirmed)
{
    if (!bConfirmed || !ReplayService || !SelectedReplay)
    {
        return;
    }

    if (US_UI_VM_ReplayEntry* Entry = Cast<US_UI_VM_ReplayEntry>(SelectedReplay))
    {
        // Set deleting state
        bIsDeleting = true;
        BroadcastDataChanged();

        ReplayService->DeleteReplay(Entry->FileName,
            [this](bool bSuccess)
            {
                bIsDeleting = false;

                if (bSuccess)
                {
                    // Refresh the list after successful deletion
                    RefreshReplays();
                }
                else
                {
                    // Show error modal
                    if (UWorld* World = GetWorld())
                    {
                        if (US_UI_Subsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
                        {
                            F_UIModalPayload Payload;
                            Payload.Message = FText::FromString(TEXT("Failed to delete the replay. Please try again."));
                            Payload.ModalType = E_UIModalType::OK;
                            UISubsystem->RequestModal(Payload, FOnModalDismissedSignature());
                        }
                    }

                    BroadcastDataChanged();
                }
            });
    }
}

FString US_UI_VM_Replays::FormatTimestamp(const FDateTime& Timestamp) const
{
    return Timestamp.ToString(TEXT("%Y-%m-%d %H:%M"));
}

FString US_UI_VM_Replays::FormatFileSize(int32 SizeKB) const
{
    if (SizeKB < 1024)
    {
        return FString::Printf(TEXT("%d KB"), SizeKB);
    }
    else
    {
        float SizeMB = SizeKB / 1024.0f;
        return FString::Printf(TEXT("%.1f MB"), SizeMB);
    }
}