// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_ModalWidget.cpp

#include "UI/S_UI_ModalWidget.h"
#include "CommonTextBlock.h"
#include "CommonButtonBase.h"
#include "UI/S_UI_TextButton.h" // Required for casting to set button text

void US_UI_ModalWidget::SetupModal(const F_UIModalPayload& Payload)
{
	if (Text_Message)
	{
		Text_Message->SetText(Payload.Message);
	}

	// Configure buttons based on modal type
	if (Btn_Confirm && Btn_Decline)
	{
		US_UI_TextButton* ConfirmButton = Cast<US_UI_TextButton>(Btn_Confirm);
		US_UI_TextButton* DeclineButton = Cast<US_UI_TextButton>(Btn_Decline);

		switch (Payload.ModalType)
		{
		case E_UIModalType::ConfirmDecline:
			Btn_Confirm->SetVisibility(ESlateVisibility::Visible);
			Btn_Decline->SetVisibility(ESlateVisibility::Visible);
			if (ConfirmButton) ConfirmButton->SetButtonText(Payload.ConfirmText);
			if (DeclineButton) DeclineButton->SetButtonText(Payload.DeclineText);
			break;

		case E_UIModalType::YesNo:
			Btn_Confirm->SetVisibility(ESlateVisibility::Visible);
			Btn_Decline->SetVisibility(ESlateVisibility::Visible);
			if (ConfirmButton) ConfirmButton->SetButtonText(FText::FromString(TEXT("Yes")));
			if (DeclineButton) DeclineButton->SetButtonText(FText::FromString(TEXT("No")));
			break;

		case E_UIModalType::OK:
			Btn_Confirm->SetVisibility(ESlateVisibility::Visible);
			Btn_Decline->SetVisibility(ESlateVisibility::Collapsed);
			if (ConfirmButton) ConfirmButton->SetButtonText(FText::FromString(TEXT("OK")));
			break;
		}
	}
}

void US_UI_ModalWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Confirm)
	{
		Btn_Confirm->OnClicked().AddUObject(this, &US_UI_ModalWidget::HandleConfirmClicked);
	}

	if (Btn_Decline)
	{
		Btn_Decline->OnClicked().AddUObject(this, &US_UI_ModalWidget::HandleDeclineClicked);
	}
}

void US_UI_ModalWidget::HandleConfirmClicked()
{
	OnDismissed.Broadcast(true);
	RemoveFromParent();
}

void US_UI_ModalWidget::HandleDeclineClicked()
{
	OnDismissed.Broadcast(false);
	RemoveFromParent();
}