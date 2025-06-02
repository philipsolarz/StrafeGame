#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "S_ViewModelBase.generated.h"

class AS_PlayerController;

/**
 * Base class for all ViewModels in the StrafeGame UI system.
 * Provides common functionality or interfaces for ViewModels.
 */
UCLASS(BlueprintType, Blueprintable) // Blueprintable if you want to create BP child ViewModels
class STRAFEGAME_API US_ViewModelBase : public UObject
{
    GENERATED_BODY()

public:
    // Initializes the ViewModel with the owning PlayerController.
    // Views or HUD Managers can call this.
    UFUNCTION(BlueprintCallable, Category = "ViewModel")
    virtual void Initialize(AS_PlayerController* InOwningPlayerController);

    // Called when the ViewModel is about to be destroyed or no longer used.
    UFUNCTION(BlueprintCallable, Category = "ViewModel")
    virtual void Deinitialize();

protected:
    // Weak pointer to the owning player controller to avoid circular dependencies
    // and to allow access to game state if needed.
    UPROPERTY(BlueprintReadOnly, Category = "ViewModel", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<AS_PlayerController> OwningPlayerController;

    // Helper to get the PlayerController safely.
    AS_PlayerController* GetOwningPlayerController() const;
};