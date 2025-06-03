// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/MainMenu/Screens/S_ReplaysScreen.h"
#include "CommonButtonBase.h"
#include "Components/ListView.h"
#include "UI/MainMenu/MenuManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

void US_ReplaysScreen::NativeConstruct()
{
    Super::NativeConstruct();

    if (Btn_PlayReplay)
    {
        Btn_PlayReplay->OnClicked().AddUObject(this, &US_ReplaysScreen::OnPlayReplayClicked);
        Btn_PlayReplay->SetIsEnabled(false);
    }
    if (Btn_DeleteReplay)
    {
        Btn_DeleteReplay->OnClicked().AddUObject(this, &US_ReplaysScreen::OnDeleteReplayClicked);
        Btn_DeleteReplay->SetIsEnabled(false);
    }
    if (Btn_Back)
    {
        Btn_Back->OnClicked().AddUObject(this, &US_ReplaysScreen::OnBackClicked);
    }
    if (ReplayListView)
    {
        ReplayListView->OnItemSelectionChanged().AddUObject(this, &US_ReplaysScreen::OnReplaySelected);
    }

    PopulateReplayList();
}

void US_ReplaysScreen::SetMenuManager_Implementation(UMenuManagerSubsystem* InMenuManager)
{
    MenuManager = InMenuManager;
}

void US_ReplaysScreen::PopulateReplayList()
{
    if (!ReplayListView) return;
    ReplayListView->ClearListItems();

    for (int32 i = 0; i < 3; ++i)
    {
        US_ReplayListItemData* Item = NewObject<US_ReplayListItemData>(this);
        Item->FileName = FString::Printf(TEXT("Replay_%d.replay"), i + 1);
        Item->Timestamp = FDateTime::UtcNow() - FTimespan::FromDays(i);
        Item->Duration = FTimespan::FromMinutes(5 + i * 2);
        ReplayListView->AddItem(Item);
    }
}

void US_ReplaysScreen::OnReplaySelected(UObject* Item)
{
    SelectedReplayData = Cast<US_ReplayListItemData>(Item);
    bool bHasSelection = SelectedReplayData != nullptr;
    if (Btn_PlayReplay) Btn_PlayReplay->SetIsEnabled(bHasSelection);
    if (Btn_DeleteReplay) Btn_DeleteReplay->SetIsEnabled(bHasSelection);
}

void US_ReplaysScreen::OnPlayReplayClicked()
{
    if (SelectedReplayData)
    {
        UE_LOG(LogTemp, Log, TEXT("Attempting to play replay: %s"), *SelectedReplayData->FileName);
        // APlayerController* PC = GetOwningPlayer();
        // if (PC)
        // {
        //    PC->ConsoleCommand(FString::Printf(TEXT("demoplay %s"), *SelectedReplayData->FileName));
        // }
    }
}

// This is the callback function that will be bound to OnConfirmDialogResultSet
void US_ReplaysScreen::HandleDeleteReplayConfirmed(bool bConfirmed)
{
    if (bConfirmed && SelectedReplayData)
    {
        UE_LOG(LogTemp, Log, TEXT("Attempting to delete replay: %s"), *SelectedReplayData->FileName);
        // TODO: Implement actual replay file deletion logic here
        ReplayListView->RemoveItem(SelectedReplayData);
        SelectedReplayData = nullptr; // Clear selection after deletion
        if (Btn_PlayReplay) Btn_PlayReplay->SetIsEnabled(false);
        if (Btn_DeleteReplay) Btn_DeleteReplay->SetIsEnabled(false);
    }
    // Unbind from the manager's delegate
    if (MenuManager)
    {
        MenuManager->OnConfirmDialogResultSet.RemoveAll(this);
    }
}


void US_ReplaysScreen::OnDeleteReplayClicked()
{
    if (SelectedReplayData && MenuManager)
    {
        // Bind to the manager's broadcast delegate for this specific action
        MenuManager->OnConfirmDialogResultSet.Clear(); // Clear previous temporary bindings
        MenuManager->OnConfirmDialogResultSet.AddUObject(this, &US_ReplaysScreen::HandleDeleteReplayConfirmed);

        MenuManager->ShowConfirmDialog(
            FText::FromString("Delete Replay"),
            FText::Format(NSLOCTEXT("Replays", "DeleteConfirmMsg", "Are you sure you want to delete {0}?"), FText::FromString(SelectedReplayData->FileName))
        );
    }
}

void US_ReplaysScreen::OnBackClicked()
{
    if (MenuManager)
    {
        MenuManager->CloseTopmostScreen();
    }
}