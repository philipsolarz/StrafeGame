// Plugins/StrafeUI/Source/StrafeUI/Public/Data/S_UI_ScreenTypes.h

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "S_UI_ScreenTypes.generated.h"

// Forward declaration
class UCommonActivatableWidget;

/**
 * Delegate broadcast when a modal dialog is dismissed. (Multicast, for binding in Blueprints/Editors)
 * @param bConfirmed - True if the user confirmed the action (e.g., clicked "Yes" or "Confirm"), false otherwise.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIModalDismissed, bool, bConfirmed);

/**
 * Delegate signature for a single-cast C++ callback when a modal is dismissed.
 * This is used for passing lambdas and other C++-only callbacks.
 */
DECLARE_DELEGATE_OneParam(FOnModalDismissedSignature, bool);


/**
 * @enum E_UIScreenId
 * @brief Uniquely identifies all primary UI content areas within the application.
 * Note: MainMenu has been removed as it is now a persistent widget, not a screen.
 */
UENUM(BlueprintType)
enum class E_UIScreenId : uint8
{
	None			UMETA(DisplayName = "None"),
	CreateGame		UMETA(DisplayName = "Create Game"),
	FindGame		UMETA(DisplayName = "Find Game"),
	Leaderboards	UMETA(DisplayName = "Leaderboards"),
	Replays			UMETA(DisplayName = "Replays"),
	Settings		UMETA(DisplayName = "Settings")
};

/**
 * @enum E_UIModalType
 * @brief Defines the button layouts and behavior for modal dialogs.
 */
UENUM(BlueprintType)
enum class E_UIModalType : uint8
{
	ConfirmDecline	UMETA(DisplayName = "Confirm / Decline"),
	YesNo			UMETA(DisplayName = "Yes / No"),
	OK				UMETA(DisplayName = "OK")
};

/**
 * @struct F_UIScreenDefinition
 * @brief Defines the properties of a UI screen, associating a screen ID with its widget class.
 * This is used to manage and instantiate UI screens throughout the system.
 */
USTRUCT(BlueprintType)
struct F_UIScreenDefinition
{
	GENERATED_BODY()

	/**
	 * The widget class to instantiate for this screen.
	 * Using TSoftClassPtr allows for lazy loading, improving performance by not loading all UI assets at startup.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Screen Definition")
	TSoftClassPtr<UCommonActivatableWidget> WidgetClass;

	/**
	 * The unique identifier for this screen.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Screen Definition")
	E_UIScreenId ScreenId;

	F_UIScreenDefinition() : ScreenId(E_UIScreenId::None) {}
};

/**
 * @struct F_UIModalPayload
 * @brief Contains all necessary data to display and handle a modal dialog.
 * This structure is passed to the UI system to create modals dynamically.
 */
USTRUCT(BlueprintType)
struct F_UIModalPayload
{
	GENERATED_BODY()

	/**
	 * The main message or question to be displayed in the modal dialog.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Modal Payload")
	FText Message;

	/**
	 * The text for the confirmation button (e.g., "Confirm", "Yes").
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Modal Payload")
	FText ConfirmText;

	/**
	 * The text for the decline button (e.g., "Decline", "No").
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Modal Payload")
	FText DeclineText;

	/**
	 * The layout and button set for the modal (e.g., Yes/No, OK).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Modal Payload")
	E_UIModalType ModalType;

	F_UIModalPayload() : ModalType(E_UIModalType::OK) {}
};