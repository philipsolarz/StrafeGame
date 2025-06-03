// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MenuScreenInterface.generated.h"

class AS_MainMenuPlayerController; // Forward declaration

UINTERFACE(MinimalAPI, Blueprintable)
class UMenuScreenInterface : public UInterface
{
    GENERATED_BODY()
};

class STRAFEGAME_API IMenuScreenInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Menu Screen")
    void SetMainMenuPlayerController(AS_MainMenuPlayerController* InPlayerController);
};