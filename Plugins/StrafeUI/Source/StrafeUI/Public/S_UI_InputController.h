// Plugins/StrafeUI/Source/StrafeUI/Public/S_UI_InputController.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/StreamableManager.h" // For FStreamableHandle
#include "S_UI_InputController.generated.h"

// Forward declarations
class US_UI_Subsystem;
class UEnhancedInputComponent;
class UInputAction;
class US_UI_Settings;
struct FInputActionValue;

/**
 * @class US_UI_InputController
 * @brief Manages UI-specific input, binding actions to handler functions.
 *
 * This object is owned by the S_UI_Subsystem and is responsible for all
 * UI-related input processing, such as navigation, accept, and back actions.
 */
UCLASS()
class STRAFEUI_API US_UI_InputController : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Initializes the input controller, binding actions to the provided input component.
	 * @param InSubsystem The owner UI subsystem.
	 * @param InputComponent The Enhanced Input component to bind actions to.
	 * @param Settings The settings object containing the Input Action assets.
	 */
	void Initialize(US_UI_Subsystem* InSubsystem, UEnhancedInputComponent* InputComponent, const US_UI_Settings* Settings);

private:
	/** Handler for the Navigate input action. */
	void OnNavigate(const FInputActionValue& Value);

	/** Handler for the Accept/Confirm input action. */
	void OnAccept(const FInputActionValue& Value);

	/** Handler for the Back/Cancel input action. */
	void OnBack(const FInputActionValue& Value);

	/** Cached pointer to the UI Subsystem. */
	UPROPERTY()
	TObjectPtr<US_UI_Subsystem> UISubsystem;

	/** Handle to the asynchronous loading of input actions. */
	TSharedPtr<FStreamableHandle> InputActionsHandle;
};