// Source/StrafeGame/Private/UI/MainMenu/MenuManagerSubsystem.cpp
#include "UI/MainMenu/MenuManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "CommonUserWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h" // Correct include
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
    ActiveWidgetStack = nullptr; // UObject will be GC'd
    MainMenuScreenInstance = nullptr;
    Super::Deinitialize();
}

void UMenuManagerSubsystem::ToggleMainMenu()
{
    APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
    if (!PC) return;

    if (bIsMenuOpen)
    {
        // Deactivate all widgets in the custom stack if main menu is closing
        if (ActiveWidgetStack)
        {
            UCommonActivatableWidget* CurrentActiveSubScreen = ActiveWidgetStack->GetActiveWidget();
            while (CurrentActiveSubScreen)
            {
                CurrentActiveSubScreen->DeactivateWidget();
                // Deactivating should pop it from the stack if the stack is configured to do so,
                // or the UCommonActivatableWidgetStack might need explicit removal if it was manually added.
                // For now, assuming DeactivateWidget is enough to hide it and allow underlying UI to show.
                // If widgets need to be fully removed from the container:
                // ActiveWidgetStack->RemoveWidget(*CurrentActiveSubScreen);
                CurrentActiveSubScreen = ActiveWidgetStack->GetActiveWidget(); // Check if another became active
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

        // If the ActiveWidgetStack is meant to be *part* of the MainMenuScreenInstance (e.g., a BindWidget UPROPERTY),
        // then MainMenuScreenInstance should expose a getter, and this subsystem would use that.
        // For now, this ActiveWidgetStack is independent and managed by the subsystem,
        // but it needs to be added to the viewport or a container within MainMenuScreenInstance to be visible.
        // A common pattern is for MainMenuScreen itself to own the primary UCommonActivatableWidgetStack.

        if (MainMenuScreenInstance)
        {
            if (MainMenuScreenInstance->GetClass()->ImplementsInterface(UMenuScreenInterface::StaticClass()))
            {
                IMenuScreenInterface::Execute_SetMenuManager(MainMenuScreenInstance, this);
            }
            MainMenuScreenInstance->ActivateWidget(); // This adds to viewport

            // If ActiveWidgetStack is for sub-screens and needs to be part of MainMenuScreen's hierarchy:
            // This step depends on WBP_MainMenuScreen having a placeholder (e.g. UOverlaySlot) for this stack.
            // For example: MainMenuScreenInstance->AddSubScreenStack(ActiveWidgetStack); (custom function)
            // Or, it could be that MainMenuScreenInstance creates ITS OWN stack, and this subsystem
            // would operate on that stack via an accessor.
            // For now, the ActiveWidgetStack created here is for screens shown *by* the manager.

            bIsMenuOpen = true;
            PC->SetInputMode(FInputModeGameAndUI());
            PC->SetShowMouseCursor(true);

            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
            {
                if (MenuInputMappingContext.IsValid())
                {
                    Subsystem->AddMappingContext(MenuInputMappingContext.LoadSynchronous(), 1); // Higher priority
                }
            }
            UE_LOG(LogTemp, Log, TEXT("Main Menu Opened."));
        }
    }
}

UCommonActivatableWidgetStack* UMenuManagerSubsystem::GetActiveWidgetStack() const
{
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
        ToggleMainMenu(); // Ensure main menu (and its potential container for ActiveWidgetStack) is open
        if (!bIsMenuOpen)
        {
            UE_LOG(LogTemp, Warning, TEXT("UMenuManagerSubsystem::ShowScreen - Main menu could not be opened. Aborting showing screen %s"), *ScreenWidgetClass->GetName());
            return;
        }
    }

    // This adds the widget to the stack and typically activates it.
    UCommonActivatableWidget* NewScreen = ActiveWidgetStack->AddWidget<UCommonActivatableWidget>(ScreenWidgetClass);
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
        ActiveWidgetStack->GetActiveWidget()->DeactivateWidget(); // Deactivating should pop it if stack is configured correctly
        UE_LOG(LogTemp, Log, TEXT("Closing topmost screen by deactivating."));
    }
    else if (bIsMenuOpen && MainMenuScreenInstance && MainMenuScreenInstance->IsActivated())
    {
        // If no "sub-screen" is active on our stack, then "close topmost" means close the main menu itself.
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
        DialogWidget->OnDialogClosedDelegate.Clear(); // Clear any previous bindings
        DialogWidget->OnDialogClosedDelegate.AddDynamic(this, &UMenuManagerSubsystem::HandleConfirmDialogClosedInternal);
        DialogWidget->SetupDialog(Title, Message);

        // Activate the dialog. It should handle its own presentation (e.g., adding to viewport).
        // CommonUI modal dialogs are often pushed to a specific layer.
        // If ActiveWidgetStack is the general menu stack:
        if (ActiveWidgetStack)
        {
            ActiveWidgetStack->AddWidget(DialogWidget); // This will activate it on top
        }
        else
        {
            DialogWidget->ActivateWidget(); // Fallback if no specific stack
        }
        UE_LOG(LogTemp, Log, TEXT("Confirm Dialog Shown."));
    }
    else
    {
        OnConfirmDialogResultSet.Broadcast(false);
    }
}

// Now a UFUNCTION
void UMenuManagerSubsystem::HandleConfirmDialogClosedInternal(bool bConfirmed)
{
    OnConfirmDialogResultSet.Broadcast(bConfirmed);
    UE_LOG(LogTemp, Log, TEXT("Confirm Dialog Closed. Confirmed: %d"), bConfirmed);
    // The dialog should remove itself from the stack or viewport upon deactivation.
}