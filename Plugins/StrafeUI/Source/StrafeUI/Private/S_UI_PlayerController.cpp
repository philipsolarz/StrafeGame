// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_PlayerController.cpp

#include "S_UI_PlayerController.h"
#include "S_UI_Subsystem.h" 
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerState.h"

void AS_UI_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	// FIX: Only initialize the UI for local players.
	// A server's copy of a player controller for a client is not a local controller.
	if (IsLocalController())
	{
		// Set up the UI and input for the main menu.
		if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (US_UI_Subsystem* UISubsystem = LocalPlayer->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
			{
				UISubsystem->InitializeUIForPlayer(this);
			}

			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (UIInputMappingContext)
				{
					InputSubsystem->AddMappingContext(UIInputMappingContext, 0);
				}
			}
		}

		SetShowMouseCursor(true);
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
}

void AS_UI_PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// When this PlayerController is destroyed (e.g., when traveling to a new level),
	// we must clean up the input mode to ensure the next PlayerController has control.
	// It's safe to run this on all controllers, local or not.
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	SetShowMouseCursor(false);

	Super::EndPlay(EndPlayReason);
}