// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/MainMenu/MenuManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "CommonUserWidget.h"
// CommonActivatableWidgetStack.h is now included via MenuManagerSubsystem.h
#include "UI/MainMenu/Screens/S_MainMenuScreen.h"
#include "UI/MainMenu/Widgets/S_ConfirmDialogWidget.h"
#include "UI/MainMenu/MenuScreenInterface.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"

void UMenuManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bIsMenuOpen = false;

    if (GetGameInstance())
    {
        ActiveWidgetStack = NewObject<UCommonActivatableWidgetStack>(GetGameInstance(), TEXT("ActiveWidgetStack"));
    }
    UE_LOG(LogTemp, Log, TEXT("UMenuManagerSubsystem Initialized."));
}

void UMenuManagerSubsystem::Deinitialize()
{
    ActiveWidgetStack = nullptr;
    MainMenuScreenInstance = nullptr;
    Super::Deinitialize();
}

void UMenuManagerSubsystem::ToggleMainMenu()
{
    APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
    if (!PC) return;

    if (bIsMenuOpen)
    {
        if (ActiveWidgetStack)
        {
            ActiveWidgetStack->ClearWidgets();
        }
        if (MainMenuScreenInstance && MainMenuScreenInstance->IsActivated())
        {
            MainMenuScreenInstance->DeactivateWidget();
        }
        bIsMenuOpen = false;
        PC->SetInputMode(FInputModeGameOnly());
        PC->SetShowMouseCursor(false);

        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (MenuInputMappingContext.IsValid())
            {
                Subsystem->RemoveMappingContext(MenuInputMappingContext.LoadSynchronous());
            }
        }
        UE_LOG(LogTemp, Log, TEXT("Main Menu Closed."));
    }
    else
    {
        if (MainMenuScreenClass.IsNull())
        {
            UE_LOG(LogTemp, Error, TEXT("MainMenuScreenClass not set in MenuManagerSubsystem!"));
            return;
        }

        TSubclassOf<US_MainMenuScreen> LoadedMainMenuClass = MainMenuScreenClass.LoadSynchronous();
        if (!LoadedMainMenuClass)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load MainMenuScreenClass!"));
            return;
        }

        if (!MainMenuScreenInstance)
        {
            MainMenuScreenInstance = CreateWidget<US_MainMenuScreen>(PC, LoadedMainMenuClass);
        }

        if (MainMenuScreenInstance)
        {
            if (MainMenuScreenInstance->GetClass()->ImplementsInterface(UMenuScreenInterface::StaticClass()))
            {
                IMenuScreenInterface::Execute_SetMenuManager(MainMenuScreenInstance, this);
            }
            MainMenuScreenInstance->ActivateWidget();
            bIsMenuOpen = true;
            PC->SetInputMode(FInputModeUIOnly());
            PC->SetShowMouseCursor(true);

            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
            {
                if (MenuInputMappingContext.IsValid())
                {
                    Subsystem->AddMappingContext(MenuInputMappingContext.LoadSynchronous(), 1);
                }
            }
            UE_LOG(LogTemp, Log, TEXT("Main Menu Opened."));
        }
    }
}

void UMenuManagerSubsystem::ShowScreen(TSubclassOf<UCommonActivatableWidget> ScreenWidgetClass)
{
    if (!ScreenWidgetClass || !ActiveWidgetStack) return;

    APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
    if (!PC) return;

    if (!bIsMenuOpen)
    {
        ToggleMainMenu();
        if (!bIsMenuOpen)
        {
            UE_LOG(LogTemp, Warning, TEXT("UMenuManagerSubsystem::ShowScreen - Main menu could not be opened. Aborting showing screen %s"), *ScreenWidgetClass->GetName());
            return;
        }
    }

    UCommonActivatableWidget* NewScreen = ActiveWidgetStack->PushWidget<UCommonActivatableWidget>(ScreenWidgetClass);
    if (NewScreen)
    {
        if (NewScreen->GetClass()->ImplementsInterface(UMenuScreenInterface::StaticClass()))
        {
            IMenuScreenInterface::Execute_SetMenuManager(NewScreen, this);
        }
        UE_LOG(LogTemp, Log, TEXT("Showing screen: %s"), *ScreenWidgetClass->GetName());
    }
}

void UMenuManagerSubsystem::CloseTopmostScreen()
{
    if (ActiveWidgetStack && ActiveWidgetStack->GetWidgetCount() > 0)
    {
        ActiveWidgetStack->PopWidget();
        UE_LOG(LogTemp, Log, TEXT("Closing topmost screen."));
    }
    else if (bIsMenuOpen)
    {
        ToggleMainMenu();
    }
}

void UMenuManagerSubsystem::ShowConfirmDialog(const FText& Title, const FText& Message)
{
    if (ConfirmDialogClass.IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("ConfirmDialogClass not set in MenuManagerSubsystem!"));
        OnConfirmDialogResultSet.Broadcast(false);
        return;
    }

    TSubclassOf<US_ConfirmDialogWidget> LoadedDialogClass = ConfirmDialogClass.LoadSynchronous();
    if (!LoadedDialogClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load ConfirmDialogClass!"));
        OnConfirmDialogResultSet.Broadcast(false);
        return;
    }

    APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
    if (!PC)
    {
        OnConfirmDialogResultSet.Broadcast(false);
        return;
    }

    US_ConfirmDialogWidget* DialogWidget = CreateWidget<US_ConfirmDialogWidget>(PC, LoadedDialogClass);
    if (DialogWidget)
    {
        DialogWidget->OnDialogClosedDelegate.AddUObject(this, &UMenuManagerSubsystem::HandleConfirmDialogClosedInternal);

        DialogWidget->SetupDialog(Title, Message);
        DialogWidget->ActivateWidget();
    }
    else
    {
        OnConfirmDialogResultSet.Broadcast(false);
    }
}

void UMenuManagerSubsystem::HandleConfirmDialogClosedInternal(bool bConfirmed)
{
    OnConfirmDialogResultSet.Broadcast(bConfirmed);
}