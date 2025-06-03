// Source/StrafeGame/Private/UI/MainMenu/MenuManagerSubsystem.cpp
#include "UI/MainMenu/MenuManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "CommonUserWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
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
        // Ensure ActiveWidgetStack is created correctly if it's meant to be a UCommonActivatableWidgetStack
        // If MainMenuScreenInstance is supposed to *contain* this stack, this logic might need adjustment
        // For now, assuming an independent stack managed by the subsystem.
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
            UCommonActivatableWidget* CurrentActiveSubScreen = ActiveWidgetStack->GetActiveWidget();
            while (CurrentActiveSubScreen)
            {
                CurrentActiveSubScreen->DeactivateWidget();
                CurrentActiveSubScreen = ActiveWidgetStack->GetActiveWidget();
            }
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
            PC->SetInputMode(FInputModeGameAndUI());
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

UCommonActivatableWidgetStack* UMenuManagerSubsystem::GetActiveWidgetStack() const
{
    // If your MainMenuScreenInstance is supposed to *own* the stack for sub-screens,
    // you might fetch it from there instead.
    // e.g., if (MainMenuScreenInstance) return MainMenuScreenInstance->GetSubScreenStack();
    return ActiveWidgetStack;
}

void UMenuManagerSubsystem::ShowScreen(TSubclassOf<UCommonActivatableWidget> ScreenWidgetClass)
{
    if (!ScreenWidgetClass || !ActiveWidgetStack)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMenuManagerSubsystem::ShowScreen - ScreenWidgetClass or ActiveWidgetStack is null."));
        return;
    }

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

    // Ensure the ActiveWidgetStack is part of the UI hierarchy if it's not already.
    // This might involve adding it to the MainMenuScreenInstance if it's designed to host this stack.
    // For example, if MainMenuScreen has a UOverlaySlot named "SubScreenHostSlot":
    // if (MainMenuScreenInstance && !ActiveWidgetStack->GetParent()) {
    // MainMenuScreenInstance->AddChildToOverlay(ActiveWidgetStack, FName("SubScreenHostSlot")); // Custom function hypothetical
    // }


    UCommonActivatableWidget* NewScreen = ActiveWidgetStack->AddWidget(ScreenWidgetClass);
    if (NewScreen)
    {
        if (NewScreen->GetClass()->ImplementsInterface(UMenuScreenInterface::StaticClass()))
        {
            IMenuScreenInterface::Execute_SetMenuManager(NewScreen, this);
        }
        UE_LOG(LogTemp, Log, TEXT("Showing screen: %s"), *ScreenWidgetClass->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UMenuManagerSubsystem::ShowScreen - Failed to AddWidget %s to ActiveWidgetStack"), *ScreenWidgetClass->GetName());
    }
}

void UMenuManagerSubsystem::CloseTopmostScreen()
{
    if (ActiveWidgetStack && ActiveWidgetStack->GetActiveWidget())
    {
        ActiveWidgetStack->GetActiveWidget()->DeactivateWidget();
        UE_LOG(LogTemp, Log, TEXT("Closing topmost screen by deactivating."));
    }
    else if (bIsMenuOpen && MainMenuScreenInstance && MainMenuScreenInstance->IsActivated())
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

    // Dialogs are typically pushed to a specific layer in Common UI, often managed by a UCommonUILayerRouter.
    // For simplicity here, we'll add it to our ActiveWidgetStack.
    // If you have a dedicated Dialogs layer in your Common UI setup, use that instead.
    if (ActiveWidgetStack)
    {
        // Create an instance and then push it.
        // The AddWidget on the stack that takes a TSubclassOf is preferred.
        UCommonActivatableWidget* DialogInstance = ActiveWidgetStack->AddWidget(LoadedDialogClass);
        US_ConfirmDialogWidget* DialogWidget = Cast<US_ConfirmDialogWidget>(DialogInstance);

        if (DialogWidget)
        {
            DialogWidget->OnDialogClosedDelegate.Clear();
            DialogWidget->OnDialogClosedDelegate.AddDynamic(this, &UMenuManagerSubsystem::HandleConfirmDialogClosedInternal);
            DialogWidget->SetupDialog(Title, Message);
            // Activation is handled by AddWidget if the stack is configured to activate new widgets.
            UE_LOG(LogTemp, Log, TEXT("Confirm Dialog Shown and added to ActiveWidgetStack."));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to cast dialog instance to US_ConfirmDialogWidget or AddWidget failed."));
            OnConfirmDialogResultSet.Broadcast(false);
        }
    }
    else
    {
        // Fallback if no ActiveWidgetStack (though there should be one)
        US_ConfirmDialogWidget* DialogWidget = CreateWidget<US_ConfirmDialogWidget>(PC, LoadedDialogClass);
        if (DialogWidget)
        {
            DialogWidget->OnDialogClosedDelegate.Clear();
            DialogWidget->OnDialogClosedDelegate.AddDynamic(this, &UMenuManagerSubsystem::HandleConfirmDialogClosedInternal);
            DialogWidget->SetupDialog(Title, Message);
            DialogWidget->ActivateWidget(); // Manually activate if not using a stack
            UE_LOG(LogTemp, Log, TEXT("Confirm Dialog Shown (fallback activation)."));
        }
        else
        {
            OnConfirmDialogResultSet.Broadcast(false);
        }
    }
}

void UMenuManagerSubsystem::HandleConfirmDialogClosedInternal(bool bConfirmed)
{
    OnConfirmDialogResultSet.Broadcast(bConfirmed);
    UE_LOG(LogTemp, Log, TEXT("Confirm Dialog Closed. Confirmed: %d"), bConfirmed);
}