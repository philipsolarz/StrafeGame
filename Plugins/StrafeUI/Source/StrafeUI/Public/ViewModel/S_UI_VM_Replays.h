// Plugins/StrafeUI/Source/StrafeUI/Public/ViewModel/S_UI_VM_Replays.h

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/S_UI_ViewModelBase.h"
#include "Services/S_ReplayService.h"
#include "S_UI_VM_Replays.generated.h"

/**
 * ViewModel entry for replay list items
 */
UCLASS()
class STRAFEUI_API US_UI_VM_ReplayEntry : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly)
    FString FileName;

    UPROPERTY(BlueprintReadOnly)
    FString FormattedTimestamp;

    UPROPERTY(BlueprintReadOnly)
    FString FileSizeText;

    UPROPERTY(BlueprintReadOnly)
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadOnly)
    int32 FileSizeKB;
};

/**
 * ViewModel for the Replays screen
 */
UCLASS(BlueprintType)
class STRAFEUI_API US_UI_VM_Replays : public US_UI_ViewModelBase
{
    GENERATED_BODY()

public:
    /** Initializes the ViewModel */
    void Initialize();

    /** Refreshes the replay list */
    UFUNCTION(BlueprintCallable, Category = "Replays")
    void RefreshReplays();

    /** Plays the selected replay */
    UFUNCTION(BlueprintCallable, Category = "Replays")
    void PlaySelectedReplay(APlayerController* PC);

    /** Deletes the selected replay with confirmation */
    UFUNCTION(BlueprintCallable, Category = "Replays")
    void DeleteSelectedReplay();

    /** Sets the selected replay */
    UFUNCTION(BlueprintCallable, Category = "Replays")
    void SetSelectedReplay(UObject* ReplayEntry);

    /** List of replay entries */
    UPROPERTY(BlueprintReadOnly, Category = "Replays")
    TArray<UObject*> ReplayEntries;

    /** Currently selected replay */
    UPROPERTY(BlueprintReadOnly, Category = "Replays")
    UObject* SelectedReplay;

    /** Whether data is currently being loaded */
    UPROPERTY(BlueprintReadOnly, Category = "Replays")
    bool bIsLoading;

    /** Whether the delete operation is in progress */
    UPROPERTY(BlueprintReadOnly, Category = "Replays")
    bool bIsDeleting;

private:
    /** Cached replay service */
    UPROPERTY()
    TObjectPtr<US_ReplayService> ReplayService;

    /** Formats the timestamp for display */
    FString FormatTimestamp(const FDateTime& Timestamp) const;

    /** Formats the file size for display */
    FString FormatFileSize(int32 SizeKB) const;

    /** Actually performs the deletion after confirmation */
    // <<< FIX: Added bConfirmed parameter
    void PerformDeleteReplay(bool bConfirmed);
};