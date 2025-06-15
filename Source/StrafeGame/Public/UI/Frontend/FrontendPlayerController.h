// Philip Solarz All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FrontendPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class STRAFEGAME_API AFrontendPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	//~Begin APlayerController Interface
	virtual void OnPossess(APawn* aPawn) override;
	//~End APlayerController Interface
	
};
