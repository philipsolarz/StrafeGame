// Source/StrafeGame/Private/UI/MainMenu/Screens/S_ReplaysScreen.cpp
#include "UI/MainMenu/Screens/S_ReplaysScreen.h"
#include "CommonButtonBase.h"
#include "Components/ListView.h"
#include "UI/MainMenu/S_MainMenuPlayerController.h" // Changed
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
    if (Btn_Back) Btn_Back->OnClicked().AddUObject(this, &US_ReplaysScreen::OnBackClicked);
    if (ReplayListView) ReplayListView->OnItemSelectionChanged().AddUObject(this, &US_ReplaysScreen::OnReplaySelected);
    PopulateReplayList();
}

void US_ReplaysScreen::SetMainMenuPlayerController_Implementation(AS_MainMenuPlayerController* InPlayerController)
{
    OwningMainMenuPlayerController = InPlayerController;
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
        APlayerController* PC = GetOwningPlayer();
        if (PC) PC->ConsoleCommand(FString::Printf(TEXT("demoplay %s"), *SelectedReplayData->FileName));
    }
}

void US_ReplaysScreen::HandleDeleteReplayConfirmed(bool bConfirmed)
{
    if (bConfirmed && SelectedReplayData)
    {
        UE_LOG(LogTemp, Log, TEXT("Attempting to delete replay: %s"), *SelectedReplayData->FileName);
        if (ReplayListView) ReplayListView->RemoveItem(SelectedReplayData);
        SelectedReplayData = nullptr;
        if (Btn_PlayReplay) Btn_PlayReplay->SetIsEnabled(false);
        if (Btn_DeleteReplay) Btn_DeleteReplay->SetIsEnabled(false);
    }
    if (OwningMainMenuPlayerController.IsValid())
    {
        OwningMainMenuPlayerController->OnConfirmDialogResultSet.RemoveDynamic(this, &US_ReplaysScreen::HandleDeleteReplayConfirmed);
    }
}

void US_ReplaysScreen::OnDeleteReplayClicked()
{
    if (SelectedReplayData && OwningMainMenuPlayerController.IsValid())
    {
        OwningMainMenuPlayerController->OnConfirmDialogResultSet.Clear();
        OwningMainMenuPlayerController->OnConfirmDialogResultSet.AddDynamic(this, &US_ReplaysScreen::HandleDeleteReplayConfirmed);
        OwningMainMenuPlayerController->ShowConfirmDialog(
            FText::FromString("Delete Replay"),
            FText::Format(NSLOCTEXT("ReplaysScreen", "DeleteConfirmMsgFmt", "Are you sure you want to delete {0}?"), FText::FromString(SelectedReplayData->FileName))
        );
    }
}

void US_ReplaysScreen::OnBackClicked()
{
    if (OwningMainMenuPlayerController.IsValid())
    {
        OwningMainMenuPlayerController->CloseTopmostScreen();
    }
}