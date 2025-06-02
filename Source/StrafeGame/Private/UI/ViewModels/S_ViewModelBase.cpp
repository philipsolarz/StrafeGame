#include "UI/ViewModels/S_ViewModelBase.h"
#include "Player/S_PlayerController.h" // Include for AS_PlayerController

void US_ViewModelBase::Initialize(AS_PlayerController* InOwningPlayerController)
{
    OwningPlayerController = InOwningPlayerController;
    // Base initialization logic for all ViewModels
}

void US_ViewModelBase::Deinitialize()
{
    // Base deinitialization logic
    OwningPlayerController.Reset();
}

AS_PlayerController* US_ViewModelBase::GetOwningPlayerController() const
{
    return OwningPlayerController.Get();
}