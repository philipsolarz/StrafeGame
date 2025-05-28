#pragma once

#include "CoreMinimal.h"
#include "Items/Pickups/S_Pickup.h"
#include "GameplayEffect.h" // For TSubclassOf<UGameplayEffect>
#include "GameplayEffectTypes.h" // For FGameplayAttribute (though GE handles attribute targeting)
#include "S_AmmoPickup.generated.h"

class UGameplayEffect;

/**
 * A pickup that grants a specific type and amount of ammunition to the character.
 * This is typically achieved by applying a GameplayEffect.
 */
UCLASS(Blueprintable)
class STRAFEGAME_API AS_AmmoPickup : public AS_Pickup
{
    GENERATED_BODY()

public:
    AS_AmmoPickup();

    /**
     * The GameplayEffect to apply to the character when this ammo pickup is collected.
     * This GameplayEffect should be configured to:
     * 1. Target the correct FGameplayAttribute for the ammo type (e.g., RocketAmmo, ShotgunAmmo).
     * 2. Use a EGameplayModOp::Additive modifier.
     * 3. Have a magnitude representing the amount of ammo to grant.
     * 4. Optionally, it could also affect the MaxAmmo attribute if this pickup type can increase capacity,
     * or it might rely on a separate effect for that.
     * This effect is applied on the server.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmmoPickup")
    TSubclassOf<UGameplayEffect> AmmoGrantEffect;

    /**
     * (Optional) The specific ammo attribute this pickup affects.
     * Used primarily for the CanBePickedUp check to see if the player is already at max ammo.
     * The AmmoGrantEffect should already target this attribute.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmmoPickup", meta = (Tooltip = "Specify the ammo attribute this pickup refills for CanBePickedUp check."))
    FGameplayAttribute AmmoAttributeToRefill;

protected:
    //~ Begin AS_Pickup Interface
    /** Applies the AmmoGrantEffect to the character. */
    virtual void GivePickupTo_Implementation(AS_Character* Picker) override;

    /**
     * Checks if the character can pick up this ammo.
     * Returns false if AmmoAttributeToRefill is valid and the character is already at max ammo for that type.
     */
    virtual bool CanBePickedUp_Implementation(AS_Character* Picker) const override;
    //~ End AS_Pickup Interface
};