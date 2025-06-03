// Source/StrafeGame/Private/UI/MainMenu/S_MainMenuPlayerController.cpp
#include "UI/MainMenu/S_MainMenuPlayerController.h"
#include "UI/MainMenu/Screens/S_MainMenuScreen.h"
#include "UI/MainMenu/Widgets/S_ConfirmDialogWidget.h"
#include "UI/MainMenu/MenuScreenInterface.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "Widgets/CommonActivatableWidgetContainer.h"


AS_MainMenuPlayerController::AS_MainMenuPlayerController()
{
    // Constructor
}

void AS_MainMenuPlayerController::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController::BeginPlay for %s"), *GetNameSafe(this));

    if (IsLocalController())
    {
        ShowInitialMenu();

        FInputModeUIOnly InputModeData;
        InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(InputModeData);
        SetShowMouseCursor(true);
    }
}

void AS_MainMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CloseFullMenu(); // Ensure menu is cleaned up
    Super::EndPlay(EndPlayReason);
}

void AS_MainMenuPlayerController::ShowInitialMenu()
{
    if (MainMenuScreenInstance && MainMenuScreenInstance->GetOwningPlayer() == this && MainMenuScreenInstance->IsActivated())
    {
        UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController::ShowInitialMenu - Main menu already active."));
        return;
    }

    if (MainMenuScreenClass.IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("MainMenuScreenClass not set in AS_MainMenuPlayerController!"));
        return;
    }

    TSubclassOf<US_MainMenuScreen> LoadedMainMenuClass = MainMenuScreenClass.LoadSynchronous();
    if (!LoadedMainMenuClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load MainMenuScreenClass!"));
        return;
    }

    if (MainMenuScreenInstance) // Should be cleaned up by EndPlay or CloseFullMenu if applicable
    {
        MainMenuScreenInstance->RemoveFromParent();
        MainMenuScreenInstance->MarkAsGarbage();
        MainMenuScreenInstance = nullptr;
    }

    MainMenuScreenInstance = CreateWidget<US_MainMenuScreen>(this, LoadedMainMenuClass);

    if (MainMenuScreenInstance)
    {
        if (MainMenuScreenInstance->GetClass()->ImplementsInterface(UMenuScreenInterface::StaticClass()))
        {
            IMenuScreenInterface::Execute_SetMainMenuPlayerController(MainMenuScreenInstance, this);
        }

        // ADD THIS LINE - Add to viewport before activating
        MainMenuScreenInstance->AddToViewport();

        MainMenuScreenInstance->ActivateWidget();
        UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController: Main Menu Screen Activated."));

        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            if (MenuInputMappingContext.IsValid())
            {
                Subsystem->AddMappingContext(MenuInputMappingContext.LoadSynchronous(), 1); // Higher priority
                UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController: Applied Menu Input Mapping Context."));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_MainMenuPlayerController: Failed to create MainMenuScreenInstance."));
    }
}

void AS_MainMenuPlayerController::CloseFullMenu()
{
    if (MainMenuScreenInstance && MainMenuScreenInstance->GetOwningPlayer() == this)
    {
        // First deactivate all widgets in the stack
        if (UCommonActivatableWidgetStack* Stack = MainMenuScreenInstance->GetPrimaryWidgetStack())
        {
            while (Stack->GetActiveWidget())
            {
                Stack->GetActiveWidget()->DeactivateWidget();
            }
        }

        // Then deactivate the main menu itself
        if (MainMenuScreenInstance->IsActivated())
        {
            MainMenuScreenInstance->DeactivateWidget();
        }

        // Remove from viewport
        MainMenuScreenInstance->RemoveFromParent();

        MainMenuScreenInstance = nullptr; // Allow it to be GC'd

        UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController: Main Menu Screen Deactivated and cleared."));

        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            if (MenuInputMappingContext.IsValid())
            {
                Subsystem->RemoveMappingContext(MenuInputMappingContext.LoadSynchronous());
                UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController: Removed Menu Input Mapping Context."));
            }
        }
        // Restore game input mode if needed by game logic after menu closes
       // FInputModeGameOnly InputModeData;
       // SetInputMode(InputModeData);
       // SetShowMouseCursor(false);
    }
}

void AS_MainMenuPlayerController::ShowScreen(TSubclassOf<UCommonActivatableWidget> ScreenWidgetClass)
{
    if (!MainMenuScreenInstance || !MainMenuScreenInstance->IsActivated())
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_MainMenuPlayerController::ShowScreen - MainMenuScreenInstance is not active. Cannot show sub-screen %s"), *ScreenWidgetClass->GetName());
        ShowInitialMenu(); // Attempt to re-initialize if not active
        if (!MainMenuScreenInstance || !MainMenuScreenInstance->IsActivated()) return;
    }

    if (ScreenWidgetClass)
    {
        MainMenuScreenInstance->PushScreenToStack(ScreenWidgetClass);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_MainMenuPlayerController::ShowScreen - ScreenWidgetClass is null."));
    }
}

void AS_MainMenuPlayerController::CloseTopmostScreen()
{
    if (MainMenuScreenInstance && MainMenuScreenInstance->IsActivated())
    {
        UCommonActivatableWidgetStack* Stack = MainMenuScreenInstance->GetPrimaryWidgetStack();
        if (Stack && Stack->GetActiveWidget())
        {
            MainMenuScreenInstance->PopScreenFromStack();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_MainMenuPlayerController::CloseTopmostScreen - MainMenuScreenInstance not active."));
    }
}

void AS_MainMenuPlayerController::ShowConfirmDialog(const FText& Title, const FText& Message)
{
    if (ConfirmDialogClass.IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("ConfirmDialogClass not set in AS_MainMenuPlayerController!"));
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

    if (!MainMenuScreenInstance || !MainMenuScreenInstance->IsActivated())
    {
        ShowInitialMenu();
        if (!MainMenuScreenInstance || !MainMenuScreenInstance->IsActivated())
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot show confirm dialog, main menu screen is not available."));
            OnConfirmDialogResultSet.Broadcast(false);
            return;
        }
    }

    if (ActiveConfirmDialog)
    {
        ActiveConfirmDialog->DeactivateWidget(); // Should also remove from parent
        ActiveConfirmDialog = nullptr;
    }

    ActiveConfirmDialog = CreateWidget<US_ConfirmDialogWidget>(this, LoadedDialogClass);
    if (ActiveConfirmDialog)
    {
        ActiveConfirmDialog->OnDialogClosedDelegate.Clear();
        ActiveConfirmDialog->OnDialogClosedDelegate.AddDynamic(this, &AS_MainMenuPlayerController::HandleConfirmDialogClosedInternal);
        ActiveConfirmDialog->SetupDialog(Title, Message);

        // ADD THIS LINE - Add to viewport with a higher Z-order to ensure it appears on top
        ActiveConfirmDialog->AddToViewport(10); // 10 is a higher Z-order

        ActiveConfirmDialog->ActivateWidget();
        UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController: Confirm Dialog Shown."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create ConfirmDialogWidget instance."));
        OnConfirmDialogResultSet.Broadcast(false);
    }
}

void AS_MainMenuPlayerController::HandleConfirmDialogClosedInternal(bool bConfirmed)
{
    OnConfirmDialogResultSet.Broadcast(bConfirmed);
    if (ActiveConfirmDialog) // Dialog should deactivate itself
    {
        ActiveConfirmDialog = nullptr;
    }
    UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController: Confirm Dialog Closed. Confirmed: %d"), bConfirmed);
}