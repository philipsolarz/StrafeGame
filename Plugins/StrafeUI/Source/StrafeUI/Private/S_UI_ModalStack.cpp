// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_ModalStack.cpp

#include "S_UI_ModalStack.h"
#include "S_UI_Subsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/S_UI_ModalWidget.h"
#include "Data/S_UI_ScreenTypes.h"


void US_UI_ModalStack::Initialize(US_UI_Subsystem* InSubsystem, TSoftClassPtr<US_UI_ModalWidget> InModalWidgetClass)
{
	check(InSubsystem);
	UISubsystem = InSubsystem;
	ModalWidgetClass = InModalWidgetClass;
}

void US_UI_ModalStack::QueueModal(const F_UIModalPayload& Payload, const FOnModalDismissedSignature& OnDismissedCallback)
{
	ModalRequestQueue.Add({ Payload, OnDismissedCallback });
	TryDisplayNextModal();
}

void US_UI_ModalStack::TryDisplayNextModal()
{
    if (ActiveModal || ModalRequestQueue.Num() == 0)
    {
        return;
    }

    // The class should already be loaded when Initialize was called
    if (!ModalWidgetClass.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("ModalStack: ModalWidgetClass is not valid! This should have been set during initialization."));
        if (ModalRequestQueue.Num() > 0)
        {
            ModalRequestQueue.RemoveAt(0);
        }
        return;
    }

    const F_UIModalRequest& NextRequest = ModalRequestQueue[0];

    APlayerController* PC = UISubsystem->GetGameInstance()->GetFirstLocalPlayerController();
    if (!PC)
    {
        return;
    }

    // Use Get() since we know it's already loaded
    US_UI_ModalWidget* NewModal = CreateWidget<US_UI_ModalWidget>(PC, ModalWidgetClass.Get());
    if (NewModal)
    {
        ActiveModal = NewModal;
        ActiveModalDismissedCallback = NextRequest.OnDismissedCallback;

        NewModal->SetupModal(NextRequest.Payload);
        NewModal->OnDismissed.AddDynamic(this, &US_UI_ModalStack::OnModalDismissed);
        NewModal->AddToViewport(100); // High Z-order to ensure it's on top
    }

    ModalRequestQueue.RemoveAt(0);
}

void US_UI_ModalStack::OnModalDismissed(bool bConfirmed)
{
	if (ActiveModalDismissedCallback.IsBound())
	{
		ActiveModalDismissedCallback.Execute(bConfirmed);
	}

	if (ActiveModal)
	{
		// The widget handles its own RemoveFromParent()
		ActiveModal = nullptr;
	}

	ActiveModalDismissedCallback.Unbind();

	TryDisplayNextModal();
}