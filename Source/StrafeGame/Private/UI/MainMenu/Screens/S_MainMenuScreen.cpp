// Source/StrafeGame/Private/UI/MainMenu/Screens/S_MainMenuScreen.cpp
#include "UI/MainMenu/Screens/S_MainMenuScreen.h"
#include "CommonButtonBase.h"
#include "UI/MainMenu/MenuManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MainMenu/Screens/S_CreateGameScreen.h"
#include "UI/MainMenu/Screens/S_FindGameScreen.h"
#include "UI/MainMenu/Screens/S_ReplaysScreen.h"
#include "UI/MainMenu/Screens/S_SettingsScreen.h"

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
        MenuManager->OnConfirmDialogResultSet.Clear();
        MenuManager->OnConfirmDialogResultSet.AddDynamic(this, &US_MainMenuScreen::OnQuitGameConfirmed);
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
    if (MenuManager)
    {
        MenuManager->OnConfirmDialogResultSet.RemoveDynamic(this, &US_MainMenuScreen::OnQuitGameConfirmed);
    }
}