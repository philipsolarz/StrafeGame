// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_InputController.cpp

#include "S_UI_InputController.h"
#include "S_UI_Settings.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "EnhancedInputComponent.h"
#include "S_UI_Subsystem.h"
#include "S_UI_Navigator.h"
#include "InputAction.h" // Include for UInputAction
#include "InputActionValue.h"
#include "UI/S_UI_RootWidget.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void US_UI_InputController::Initialize(US_UI_Subsystem* InSubsystem, UEnhancedInputComponent* InputComponent, const US_UI_Settings* Settings)
{
	check(InSubsystem);
	check(InputComponent);
	check(Settings);

	UISubsystem = InSubsystem;

	// Asynchronously load the input actions.
	// This prevents hitching the game thread while waiting for assets to load.
	FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
	TArray<FSoftObjectPath> ActionsToLoad;

	if (!Settings->NavigateAction.IsNull())
	{
		ActionsToLoad.Add(Settings->NavigateAction.ToSoftObjectPath());
	}
	if (!Settings->AcceptAction.IsNull())
	{
		ActionsToLoad.Add(Settings->AcceptAction.ToSoftObjectPath());
	}
	if (!Settings->BackAction.IsNull())
	{
		ActionsToLoad.Add(Settings->BackAction.ToSoftObjectPath());
	}

	if (ActionsToLoad.Num() > 0)
	{
		// Use a weak pointer to safely access the InputComponent in the callback.
		// The InputController could be destroyed before loading finishes.
		TWeakObjectPtr<UEnhancedInputComponent> WeakInputComponent = InputComponent;
		TWeakObjectPtr<const US_UI_Settings> WeakSettings = Settings;

		InputActionsHandle = StreamableManager.RequestAsyncLoad(ActionsToLoad,
			[this, WeakInputComponent, WeakSettings]()
			{
				if (!WeakInputComponent.IsValid() || !WeakSettings.IsValid())
				{
					return; // Component or settings were destroyed before loading completed.
				}

				UEnhancedInputComponent* StrongInputComponent = WeakInputComponent.Get();
				const US_UI_Settings* StrongSettings = WeakSettings.Get();

				// Now that assets are loaded, we can bind them.
				if (UInputAction* NavigateAction = StrongSettings->NavigateAction.Get()) { StrongInputComponent->BindAction(NavigateAction, ETriggerEvent::Triggered, this, &US_UI_InputController::OnNavigate); }
				if (UInputAction* AcceptAction = StrongSettings->AcceptAction.Get()) { StrongInputComponent->BindAction(AcceptAction, ETriggerEvent::Started, this, &US_UI_InputController::OnAccept); }
				if (UInputAction* BackAction = StrongSettings->BackAction.Get()) { StrongInputComponent->BindAction(BackAction, ETriggerEvent::Started, this, &US_UI_InputController::OnBack); }
			});
	}
}

void US_UI_InputController::OnNavigate(const FInputActionValue& Value)
{
	const FVector2D NavDirection = Value.Get<FVector2D>();
	UE_LOG(LogTemp, Verbose, TEXT("InputController: Navigate triggered with value: %s"), *NavDirection.ToString());

	if (UISubsystem && UISubsystem->GetRootWidget())
	{
		// Navigation should now be routed to the active widget in the *content* stack
		if (US_UI_BaseScreenWidget* ActiveWidget = Cast<US_UI_BaseScreenWidget>(UISubsystem->GetRootWidget()->GetContentStack()->GetActiveWidget()))
		{
			ActiveWidget->HandleNavigation(NavDirection);
		}
	}
}

void US_UI_InputController::OnAccept(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Verbose, TEXT("InputController: Accept triggered."));

	if (UISubsystem && UISubsystem->GetRootWidget())
	{
		if (US_UI_BaseScreenWidget* ActiveWidget = Cast<US_UI_BaseScreenWidget>(UISubsystem->GetRootWidget()->GetContentStack()->GetActiveWidget()))
		{
			ActiveWidget->HandleAccept();
		}
	}
}

void US_UI_InputController::OnBack(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Verbose, TEXT("InputController: Back triggered."));

	if (UISubsystem && UISubsystem->GetNavigator())
	{
		// <<< Corrected call to use the Navigator
		UISubsystem->GetNavigator()->PopContentScreen();
	}
}