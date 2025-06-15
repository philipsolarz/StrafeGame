// Plugins/StrafeUI/Source/StrafeUI/Public/Data/S_UI_InputTypes.h

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "S_UI_InputTypes.generated.h"

/**
 * Defines a user-configurable key binding for an input action.
 * This is distinct from the engine's FInputActionBinding to avoid conflicts
 * and provide additional metadata for the settings UI.
 */
USTRUCT(BlueprintType)
struct FStrafeInputActionBinding
{
    GENERATED_BODY()

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
    FName ActionName;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
    FKey PrimaryKey;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
    FKey SecondaryKey;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
    FString Category;

    FStrafeInputActionBinding()
        : PrimaryKey(EKeys::Invalid)
        , SecondaryKey(EKeys::Invalid)
    {
        Category = "General";
    }

    FStrafeInputActionBinding(const FName InActionName, const FText& InDisplayName, const FKey InPrimaryKey, const FKey InSecondaryKey, const FString& InCategory)
        : ActionName(InActionName)
        , DisplayName(InDisplayName)
        , PrimaryKey(InPrimaryKey)
        , SecondaryKey(InSecondaryKey)
        , Category(InCategory)
    {
    }
};