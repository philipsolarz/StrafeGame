// Philip Solarz All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Widget_ActivatableBase.generated.h"

class AFrontendPlayerController;
/**
 * 
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class STRAFEGAME_API UWidget_ActivatableBase : public UCommonActivatableWidget
{
	GENERATED_BODY()

private:
	TWeakObjectPtr<AFrontendPlayerController> CachedOwningFrontendPlayerController;

protected:
	UFUNCTION(BlueprintPure)
	AFrontendPlayerController* GetOwningFrontendPlayerController();
};
