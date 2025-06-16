// Plugins/StrafeUI/Source/StrafeUI/Public/S_UI_PlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "S_UI_PlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class STRAFEUI_API AS_UI_PlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	/** The Input Mapping Context to use for the UI. Assign this in the Blueprint subclass. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> UIInputMappingContext;

	virtual void BeginPlay() override;

	/** Called when the controller is being destroyed. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};