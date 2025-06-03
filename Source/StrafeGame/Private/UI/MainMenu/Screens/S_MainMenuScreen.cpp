// Source/StrafeGame/Private/UI/MainMenu/Screens/S_MainMenuScreen.cpp
#include "UI/MainMenu/Screens/S_MainMenuScreen.h"
#include "CommonButtonBase.h"
#include "UI/MainMenu/S_MainMenuPlayerController.h" // Changed from MenuManagerSubsystem
#include "Kismet/GameplayStatics.h"
#include "UI/MainMenu/Screens/S_CreateGameScreen.h"
#include "UI/MainMenu/Screens/S_FindGameScreen.h"
#include "UI/MainMenu/Screens/S_ReplaysScreen.h"
#include "UI/MainMenu/Screens/S_SettingsScreen.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "UI/MainMenu/MenuScreenInterface.h"

void US_MainMenuScreen::NativeConstruct()
{
    Super::NativeConstruct();

    if (Btn_CreateGame) Btn_CreateGame->OnClicked().AddUObject(this, &US_MainMenuScreen::OnCreateGameClicked);
    if (Btn_FindGame) Btn_FindGame->OnClicked().AddUObject(this, &US_MainMenuScreen::OnFindGameClicked);
    if (Btn_Replays) Btn_Replays->OnClicked().AddUObject(this, &US_MainMenuScreen::OnReplaysClicked);
    if (Btn_Settings) Btn_Settings->OnClicked().AddUObject(this, &US_MainMenuScreen::OnSettingsClicked);
    if (Btn_QuitGame) Btn_QuitGame->OnClicked().AddUObject(this, &US_MainMenuScreen::OnQuitGameClicked);

    if (!PrimaryWidgetStack)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_MainMenuScreen: PrimaryWidgetStack is not bound in WBP_MainMenuScreen!"));
    }
}

// Updated interface implementation
void US_MainMenuScreen::SetMainMenuPlayerController_Implementation(AS_MainMenuPlayerController* InPlayerController)
{
    OwningMainMenuPlayerController = InPlayerController;
}

void US_MainMenuScreen::PushScreenToStack(TSubclassOf<UCommonActivatableWidget> ScreenWidgetClass)
{
    if (!PrimaryWidgetStack)
    {
        UE_LOG(LogTemp, Error, TEXT("US_MainMenuScreen::PushScreenToStack - PrimaryWidgetStack is null!"));
        return;
    }
    if (!ScreenWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("US_MainMenuScreen::PushScreenToStack - ScreenWidgetClass is null!"));
        return;
    }

    UCommonActivatableWidget* NewScreen = PrimaryWidgetStack->AddWidget(ScreenWidgetClass);
    if (NewScreen)
    {
        // Pass the PlayerController to the new screen if it implements the interface
        if (OwningMainMenuPlayerController.IsValid() && NewScreen->GetClass()->ImplementsInterface(UMenuScreenInterface::StaticClass()))
        {
            IMenuScreenInterface::Execute_SetMainMenuPlayerController(NewScreen, OwningMainMenuPlayerController.Get());
        }
        UE_LOG(LogTemp, Log, TEXT("US_MainMenuScreen: Pushed screen %s to PrimaryWidgetStack."), *ScreenWidgetClass->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("US_MainMenuScreen::PushScreenToStack - Failed to add widget %s to PrimaryWidgetStack."), *ScreenWidgetClass->GetName());
    }
}

void US_MainMenuScreen::PopScreenFromStack()
{
    if (PrimaryWidgetStack && PrimaryWidgetStack->GetActiveWidget())
    {
        PrimaryWidgetStack->GetActiveWidget()->DeactivateWidget();
        UE_LOG(LogTemp, Log, TEXT("US_MainMenuScreen: Popped screen from PrimaryWidgetStack."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("US_MainMenuScreen::PopScreenFromStack - No active widget on PrimaryWidgetStack to pop."));
    }
}

void US_MainMenuScreen::OnCreateGameClicked()
{
    PushScreenToStack(US_CreateGameScreen::StaticClass());
}

void US_MainMenuScreen::OnFindGameClicked()
{
    PushScreenToStack(US_FindGameScreen::StaticClass());
}

void US_MainMenuScreen::OnReplaysClicked()
{
    PushScreenToStack(US_ReplaysScreen::StaticClass());
}

void US_MainMenuScreen::OnSettingsClicked()
{
    PushScreenToStack(US_SettingsScreen::StaticClass());
}

void US_MainMenuScreen::OnQuitGameClicked()
{
    if (OwningMainMenuPlayerController.IsValid())
    {
        OwningMainMenuPlayerController->OnConfirmDialogResultSet.Clear();
        OwningMainMenuPlayerController->OnConfirmDialogResultSet.AddDynamic(this, &US_MainMenuScreen::OnQuitGameConfirmed);
        OwningMainMenuPlayerController->ShowConfirmDialog(FText::FromString("Quit Game"), FText::FromString("Are you sure you want to quit?"));
    }
}

void US_MainMenuScreen::OnQuitGameConfirmed(bool bConfirmed)
{
    if (bConfirmed)
    {
        APlayerController* PC = GetOwningPlayer(); // OwningMainMenuPlayerController could also be used here
        if (PC)
        {
            PC->ConsoleCommand("quit");
        }
    }
    if (OwningMainMenuPlayerController.IsValid())
    {
        OwningMainMenuPlayerController->OnConfirmDialogResultSet.RemoveDynamic(this, &US_MainMenuScreen::OnQuitGameConfirmed);
    }
}