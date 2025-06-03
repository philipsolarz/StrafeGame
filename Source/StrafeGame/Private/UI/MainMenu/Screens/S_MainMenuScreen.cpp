// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/MainMenu/Screens/S_MainMenuScreen.h" // Updated include
#include "CommonButtonBase.h"
#include "UI/MainMenu/MenuManagerSubsystem.h" // Updated include
#include "Kismet/GameplayStatics.h"
#include "UI/MainMenu/Screens/S_CreateGameScreen.h" // Updated include
#include "UI/MainMenu/Screens/S_FindGameScreen.h"   // Updated include
#include "UI/MainMenu/Screens/S_ReplaysScreen.h"  // Updated include
#include "UI/MainMenu/Screens/S_SettingsScreen.h" // Updated include

void US_MainMenuScreen::NativeConstruct()
{
    Super::NativeConstruct();

    if (Btn_CreateGame)
    {
        Btn_CreateGame->OnClicked().AddUObject(this, &US_MainMenuScreen::OnCreateGameClicked);
    }
    if (Btn_FindGame)
    {
        Btn_FindGame->OnClicked().AddUObject(this, &US_MainMenuScreen::OnFindGameClicked);
    }
    if (Btn_Replays)
    {
        Btn_Replays->OnClicked().AddUObject(this, &US_MainMenuScreen::OnReplaysClicked);
    }
    if (Btn_Settings)
    {
        Btn_Settings->OnClicked().AddUObject(this, &US_MainMenuScreen::OnSettingsClicked);
    }
    if (Btn_QuitGame)
    {
        Btn_QuitGame->OnClicked().AddUObject(this, &US_MainMenuScreen::OnQuitGameClicked);
    }
}

void US_MainMenuScreen::SetMenuManager_Implementation(UMenuManagerSubsystem* InMenuManager)
{
    MenuManager = InMenuManager;
}

void US_MainMenuScreen::OnCreateGameClicked()
{
    if (MenuManager)
    {
        MenuManager->ShowScreen(US_CreateGameScreen::StaticClass());
    }
}

void US_MainMenuScreen::OnFindGameClicked()
{
    if (MenuManager)
    {
        MenuManager->ShowScreen(US_FindGameScreen::StaticClass());
    }
}

void US_MainMenuScreen::OnReplaysClicked()
{
    if (MenuManager)
    {
        MenuManager->ShowScreen(US_ReplaysScreen::StaticClass());
    }
}

void US_MainMenuScreen::OnSettingsClicked()
{
    if (MenuManager)
    {
        MenuManager->ShowScreen(US_SettingsScreen::StaticClass());
    }
}

void US_MainMenuScreen::OnQuitGameClicked()
{
    if (MenuManager)
    {
        // Bind to the manager's broadcast delegate for this specific action
        MenuManager->OnConfirmDialogResultSet.Clear(); // Clear previous temporary bindings
        MenuManager->OnConfirmDialogResultSet.AddUObject(this, &US_MainMenuScreen::OnQuitGameConfirmed);
        MenuManager->ShowConfirmDialog(FText::FromString("Quit Game"), FText::FromString("Are you sure you want to quit?"));
    }
}

void US_MainMenuScreen::OnQuitGameConfirmed(bool bConfirmed)
{
    if (bConfirmed)
    {
        APlayerController* PC = GetOwningPlayer();
        if (PC)
        {
            PC->ConsoleCommand("quit");
        }
    }
    // Unbind from the manager's delegate after handling the result
    if (MenuManager)
    {
        MenuManager->OnConfirmDialogResultSet.RemoveAll(this); // Or more specific RemoveUObject if needed
    }
}