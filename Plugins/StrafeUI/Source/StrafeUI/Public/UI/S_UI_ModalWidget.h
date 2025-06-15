// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_ModalWidget.h

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Data/S_UI_ScreenTypes.h"
#include "S_UI_ModalWidget.generated.h"

// Forward Declarations
class UCommonTextBlock;
class UCommonButtonBase;

/**
 * The C++ base class for the modal dialog widget (WBP_Modal).
 * Handles the setup of text and the broadcasting of dismissal events.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_ModalWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** Delegate broadcast when the modal is dismissed by the user. */
	UPROPERTY(BlueprintAssignable, Category = "Modal")
	FOnUIModalDismissed OnDismissed;

	/**
	 * Configures the modal's text and buttons based on the provided data.
	 * @param Payload The data structure containing the message and modal type.
	 */
	void SetupModal(const F_UIModalPayload& Payload);

protected:
	virtual void NativeOnInitialized() override;

private:
	/** Handler for when the confirm button is clicked. */
	UFUNCTION()
	void HandleConfirmClicked();

	/** Handler for when the decline button is clicked. */
	UFUNCTION()
	void HandleDeclineClicked();

	/** The main text block to display the modal's message. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Message;

	/** The button to confirm the modal's action (e.g., "Yes", "OK"). */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_Confirm;

	/** The button to decline the modal's action (e.g., "No", "Cancel"). */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_Decline;
};