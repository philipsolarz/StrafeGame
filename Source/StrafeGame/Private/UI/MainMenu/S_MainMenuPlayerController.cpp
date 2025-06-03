// Source/StrafeGame/Private/UI/MainMenu/S_MainMenuPlayerController.cpp
// (Assuming your header is S_MainMenuPlayerController.h)
#include "UI/MainMenu/S_MainMenuPlayerController.h" // Adjust path if your header is different
#include "UI/MainMenu/MenuManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h" // Added for IsLocalController()

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
            UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController %s: MenuManagerSubsystem found. Calling ToggleMainMenu."), *GetNameSafe(this));
            MenuManager->ToggleMainMenu();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AS_MainMenuPlayerController %s: MenuManagerSubsystem NOT found!"), *GetNameSafe(this));
        }

        UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController %s: Setting ShowMouseCursor(true) and InputModeUIOnly."), *GetNameSafe(this));
        SetShowMouseCursor(true);
        FInputModeUIOnly InputModeData;
        InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // Or LockAlways, etc.
        SetInputMode(InputModeData);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AS_MainMenuPlayerController %s: IsLocalController() is false."), *GetNameSafe(this));
    }
}