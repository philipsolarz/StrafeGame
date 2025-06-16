// Plugins/StrafeUI/Source/StrafeUI/Public/S_UI_ModalStack.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Data/S_UI_ScreenTypes.h"
#include "S_UI_ModalStack.generated.h"

// Forward declarations
class US_UI_Subsystem;
class US_UI_ModalWidget;

/**
 * @struct F_UIModalRequest
 * @brief A struct holding all information for a queued modal dialog request.
 */
USTRUCT()
struct F_UIModalRequest
{
	GENERATED_BODY()

	UPROPERTY()
	F_UIModalPayload Payload;

	FOnModalDismissedSignature OnDismissedCallback;
};


/**
 * @class US_UI_ModalStack
 * @brief Manages a queue of modal dialogs, ensuring they are displayed one by one.
 */
UCLASS()
class STRAFEUI_API US_UI_ModalStack : public UObject
{
	GENERATED_BODY()

public:
	/** Initializes the modal stack. */
	void Initialize(US_UI_Subsystem* InSubsystem, TSoftClassPtr<US_UI_ModalWidget> InModalWidgetClass);

	/** Queues a new modal dialog request. */
	void QueueModal(const F_UIModalPayload& Payload, const FOnModalDismissedSignature& OnDismissedCallback);

private:
	/** Attempts to display the next modal from the queue if one is not already active. */
	void TryDisplayNextModal();

	/** Handles the dismissal of the active modal. */
	UFUNCTION()
	void OnModalDismissed(bool bConfirmed);

	/** The queue of modal dialogs waiting to be displayed. */
	UPROPERTY()
	TArray<F_UIModalRequest> ModalRequestQueue;

	/** Pointer to the currently displayed modal widget, if any. */
	UPROPERTY()
	TObjectPtr<US_UI_ModalWidget> ActiveModal;

	/** The last callback associated with the active modal. */
	FOnModalDismissedSignature ActiveModalDismissedCallback;

	/** Cached pointer to the UI Subsystem. */
	UPROPERTY()
	TObjectPtr<US_UI_Subsystem> UISubsystem;

	/** The widget class to use for creating modals, passed in during initialization. */
	TSoftClassPtr<US_UI_ModalWidget> ModalWidgetClass;
};