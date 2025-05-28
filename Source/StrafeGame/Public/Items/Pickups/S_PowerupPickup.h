#pragma once

#include "CoreMinimal.h"
#include "Items/Pickups/S_Pickup.h"
#include "GameplayEffect.h" // For TSubclassOf<UGameplayEffect>
#include "GameplayTagContainer.h" // For FGameplayTag
#include "S_PowerupPickup.generated.h"

class UGameplayEffect;

/**
 * A pickup that applies a GameplayEffect to the character, granting a temporary
 * or permanent power-up (e.g., speed boost, damage shield, quad damage).
 */
UCLASS(Blueprintable)
class STRAFEGAME_API AS_PowerupPickup : public AS_Pickup
{
    GENERATED_BODY()

public:
    AS_PowerupPickup();

    /**
     * The GameplayEffect class to apply to the character when this powerup is collected.
     * This GE should define the powerup's duration, attribute modifications, granted tags, etc.
     * Applied on the server.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerupPickup")
    TSubclassOf<UGameplayEffect> PowerupEffectClass;

    /**
     * (Optional) A gameplay tag that this powerup grants while active.
     * Used by CanBePickedUp to check if the character already has this specific powerup active,
     * preventing stacking if desired. The PowerupEffectClass should also grant this tag.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerupPickup", meta = (Tooltip = "If set, CanBePickedUp will check if the player already has this tag from an active effect."))
    FGameplayTag PowerupGrantedTag;

    /**
     * If true, and PowerupGrantedTag is valid, the player cannot pick this up if they
     * already have an active GameplayEffect granting PowerupGrantedTag.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PowerupPickup")
    bool bPreventStacking;

protected:
    //~ Begin AS_Pickup Interface
    /** Applies the PowerupEffectClass to the character. */
    virtual void GivePickupTo_Implementation(AS_Character* Picker) override;

    /**
     * Checks if the character can pick up this powerup.
     * Returns false if bPreventStacking is true, PowerupGrantedTag is valid,
     * and the character already has an active effect granting that tag.
     */
    virtual bool CanBePickedUp_Implementation(AS_Character* Picker) const override;
    //~ End AS_Pickup Interface
};