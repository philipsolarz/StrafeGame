// Source/StrafeGame/Private/UI/MainMenu/MenuManagerSubsystem.cpp
#include "UI/MainMenu/MenuManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "UI/MainMenu/Screens/S_MainMenuScreen.h"
#include "UI/MainMenu/Widgets/S_ConfirmDialogWidget.h"
#include "UI/MainMenu/MenuScreenInterface.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h" // May not be needed here directly
#include "InputMappingContext.h"    // For UInputMappingContext
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/CommonActivatableWidgetContainer.h" // For UCommonActivatableWidgetStack

void UMenuManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // ActiveWidgetStack is removed, as S_MainMenuScreen will manage its own.
    UE_LOG(LogTemp, Log, TEXT("UMenuManagerSubsystem Initialized."));
}

void UMenuManagerSubsystem::Deinitialize()
{
    MainMenuScreenInstance = nullptr;
    ActiveConfirmDialog = nullptr;
    Super::Deinitialize();
}

APlayerController* UMenuManagerSubsystem::GetLocalPlayerController() const
{
    // This is a simple way; for multiplayer or split-screen, you'd need a more robust way
    // or pass the controller context around.
    if (GetGameInstance())
    {
        return GetGameInstance()->GetFirstLocalPlayerController();
    }
    return nullptr;
}

void UMenuManagerSubsystem::ShowInitialMenu(APlayerController* ForPlayerController)
{
    if (!ForPlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("UMenuManagerSubsystem::ShowInitialMenu - ForPlayerController is null!"));
        return;
    }

    if (MainMenuScreenInstance && MainMenuScreenInstance->GetOwningPlayer() == ForPlayerController && MainMenuScreenInstance->IsActivated())
    {
        UE_LOG(LogTemp, Log, TEXT("UMenuManagerSubsystem::ShowInitialMenu - Main menu already active for this player."));
        return;
    }

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

    // Destroy old instance if it exists (e.g., from a different player or a previous session error)
    if (MainMenuScreenInstance)
    {
        MainMenuScreenInstance->RemoveFromParent(); // Ensure it's removed from viewport
        MainMenuScreenInstance->MarkAsGarbage();    // Let GC handle it
        MainMenuScreenInstance = nullptr;
    }

    MainMenuScreenInstance = CreateWidget<US_MainMenuScreen>(ForPlayerController, LoadedMainMenuClass);

    if (MainMenuScreenInstance)
    {
        if (MainMenuScreenInstance->GetClass()->ImplementsInterface(UMenuScreenInterface::StaticClass()))
        {
            IMenuScreenInterface::Execute_SetMenuManager(MainMenuScreenInstance, this);
        }
        MainMenuScreenInstance->ActivateWidget(); // This adds to viewport and sets up for Common UI
        UE_LOG(LogTemp, Log, TEXT("Main Menu Screen Activated."));

        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(ForPlayerController->GetLocalPlayer()))
        {
            if (MenuInputMappingContext.IsValid())
            {
                Subsystem->AddMappingContext(MenuInputMappingContext.LoadSynchronous(), 1); // Higher priority for menu
                UE_LOG(LogTemp, Log, TEXT("Applied Menu Input Mapping Context."));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create MainMenuScreenInstance."));
    }
}

void UMenuManagerSubsystem::CloseMainMenu(APlayerController* ForPlayerController)
{
    if (!ForPlayerController) return;

    if (MainMenuScreenInstance && MainMenuScreenInstance->GetOwningPlayer() == ForPlayerController && MainMenuScreenInstance->IsActivated())
    {
        // Close all sub-screens first
        if (UCommonActivatableWidgetStack* Stack = MainMenuScreenInstance->GetPrimaryWidgetStack())
        {
            while (Stack->GetActiveWidget())
            {
                Stack->GetActiveWidget()->DeactivateWidget();
            }
        }
        MainMenuScreenInstance->DeactivateWidget(); // Deactivates and removes from viewport
        // MainMenuScreenInstance = nullptr; // Don't null out immediately if you might re-show it quickly. Deinitialize will handle it.

        UE_LOG(LogTemp, Log, TEXT("Main Menu Screen Deactivated."));

        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(ForPlayerController->GetLocalPlayer()))
        {
            if (MenuInputMappingContext.IsValid())
            {
                Subsystem->RemoveMappingContext(MenuInputMappingContext.LoadSynchronous());
                UE_LOG(LogTemp, Log, TEXT("Removed Menu Input Mapping Context."));
            }
        }
        // Note: Input mode (GameOnly/UIOnly) is typically handled by the PlayerController that calls ShowInitialMenu/CloseMainMenu.
    }
}


void UMenuManagerSubsystem::ShowScreen(TSubclassOf<UCommonActivatableWidget> ScreenWidgetClass)
{
    if (!MainMenuScreenInstance || !MainMenuScreenInstance->IsActivated())
    {
        UE_LOG(LogTemp, Warning, TEXT("UMenuManagerSubsystem::ShowScreen - MainMenuScreenInstance is not active. Cannot show sub-screen %s"), *ScreenWidgetClass->GetName());
        // Optionally, try to show the main menu first if this happens unexpectedly
        // APlayerController* PC = GetLocalPlayerController();
        // if (PC) ShowInitialMenu(PC);
        // if (!MainMenuScreenInstance || !MainMenuScreenInstance->IsActivated()) return; // Re-check
        return;
    }

    if (ScreenWidgetClass)
    {
        MainMenuScreenInstance->PushScreenToStack(ScreenWidgetClass);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UMenuManagerSubsystem::ShowScreen - ScreenWidgetClass is null."));
    }
}

void UMenuManagerSubsystem::CloseTopmostScreen()
{
    if (MainMenuScreenInstance && MainMenuScreenInstance->IsActivated())
    {
        UCommonActivatableWidgetStack* Stack = MainMenuScreenInstance->GetPrimaryWidgetStack();
        if (Stack && Stack->GetActiveWidget())
        {
            MainMenuScreenInstance->PopScreenFromStack();
        }
        // If no sub-screen is active on the stack, we are at the main menu screen.
        // We don't close the MainMenuScreenInstance itself here, as it's always supposed to be shown.
        // To exit the main menu level, that would be a different flow (e.g., joining a game).
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UMenuManagerSubsystem::CloseTopmostScreen - MainMenuScreenInstance not active."));
    }
}

void UMenuManagerSubsystem::ShowConfirmDialog(const FText& Title, const FText& Message)
{
    APlayerController* PC = GetLocalPlayerController();
    if (!PC)
    {
        OnConfirmDialogResultSet.Broadcast(false); // Cannot show without a player context
        return;
    }

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

    // Ensure main menu is active to host the dialog, or handle dialogs globally if needed.
    if (!MainMenuScreenInstance || !MainMenuScreenInstance->IsActivated())
    {
        ShowInitialMenu(PC); // Try to show main menu if not already up
        if (!MainMenuScreenInstance || !MainMenuScreenInstance->IsActivated())
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot show confirm dialog, main menu screen is not available."));
            OnConfirmDialogResultSet.Broadcast(false);
            return;
        }
    }

    if (ActiveConfirmDialog) // Close previous one if any
    {
        ActiveConfirmDialog->DeactivateWidget();
        ActiveConfirmDialog = nullptr;
    }

    ActiveConfirmDialog = CreateWidget<US_ConfirmDialogWidget>(PC, LoadedDialogClass);
    if (ActiveConfirmDialog)
    {
        ActiveConfirmDialog->OnDialogClosedDelegate.Clear(); // Ensure no old bindings
        ActiveConfirmDialog->OnDialogClosedDelegate.AddDynamic(this, &UMenuManagerSubsystem::HandleConfirmDialogClosedInternal);
        ActiveConfirmDialog->SetupDialog(Title, Message);

        // Dialogs are typically pushed onto a specific layer.
        // If your PrimaryWidgetStack on MainMenuScreen is intended for ALL activatable widgets including modals:
        if (UCommonActivatableWidgetStack* Stack = MainMenuScreenInstance->GetPrimaryWidgetStack())
        {
            Stack->AddWidget(LoadedDialogClass); // This might re-create it. Better to push instance.
            // For now, let's activate it directly after creating.
// If your stack doesn't manage dialogs, activate it directly:
            ActiveConfirmDialog->ActivateWidget(); // This should add it to viewport on a high ZOrder if Common UI is set up for layers.
            UE_LOG(LogTemp, Log, TEXT("Confirm Dialog Shown."));
        }
        else {
            ActiveConfirmDialog->ActivateWidget();
            UE_LOG(LogTemp, Log, TEXT("Confirm Dialog Shown (no stack available on main menu, direct activation)."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create ConfirmDialogWidget instance."));
        OnConfirmDialogResultSet.Broadcast(false);
    }
}

void UMenuManagerSubsystem::HandleConfirmDialogClosedInternal(bool bConfirmed)
{
    OnConfirmDialogResultSet.Broadcast(bConfirmed);
    if (ActiveConfirmDialog) // The dialog would call DeactivateWidget on itself.
    {
        ActiveConfirmDialog = nullptr;
    }
    UE_LOG(LogTemp, Log, TEXT("Confirm Dialog Closed. Confirmed: %d"), bConfirmed);
}