// Philip Solarz All Rights Reserved


#include "UI/Frontend/Widgets/Widget_ActivatableBase.h"
#include "UI/Frontend/FrontendPlayerController.h"

AFrontendPlayerController* UWidget_ActivatableBase::GetOwningFrontendPlayerController()
{
    if (!CachedOwningFrontendPlayerController.IsValid())
    {
        CachedOwningFrontendPlayerController = GetOwningPlayer<AFrontendPlayerController>();
    }
    return CachedOwningFrontendPlayerController.IsValid() ? CachedOwningFrontendPlayerController.Get() : nullptr;
}
