// Source/StrafeGame/Private/UI/MainMenu/S_MainMenuPlayerController.cpp
#include "UI/MainMenu/S_MainMenuPlayerController.h"
#include "UI/MainMenu/MenuManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h" // Added for IsLocalController()
#include "EnhancedInputSubsystems.h"      // For Input Mapping Context
#include "InputMappingContext.h"        // For UInputMappingContext

void AS_MainMenuPlayerController::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController::BeginPlay for %s"), *GetNameSafe(this));

    if (IsLocalController()) // Only for the local player
    {
        UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController %s: IsLocalController() is true."), *GetNameSafe(this));
        UMenuManagerSubsystem* MenuManager = GetGameInstance()->GetSubsystem<UMenuManagerSubsystem>();
        if (MenuManager)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController %s: MenuManagerSubsystem found. Calling ShowInitialMenu."), *GetNameSafe(this));
            MenuManager->ShowInitialMenu(this); // Pass this player controller

            // Set up input mode for UI
            FInputModeUIOnly InputModeData;
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            SetInputMode(InputModeData);
            SetShowMouseCursor(true);

            // Add Menu Input Mapping Context if it's managed by MenuManagerSubsystem
            // This part depends on how MenuInputMappingContext was intended to be used.
            // If MenuManagerSubsystem handles its activation/deactivation, this might be handled there.
            // For now, assuming MenuManager handles it on showing the menu.
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AS_MainMenuPlayerController %s: MenuManagerSubsystem NOT found!"), *GetNameSafe(this));
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController %s: IsLocalController() is false."), *GetNameSafe(this));
    }
}