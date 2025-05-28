#pragma once

#include "CoreMinimal.h"
#include "Items/Pickups/S_Pickup.h"
#include "S_WeaponPickup.generated.h"

class AS_Weapon; // Forward declaration for TSubclassOf

/**
 * A pickup that grants a weapon to the character.
 * If the character already has the weapon, it may grant ammo for it instead.
 */
UCLASS(Blueprintable)
class STRAFEGAME_API AS_WeaponPickup : public AS_Pickup
{
    GENERATED_BODY()

public:
    AS_WeaponPickup();

    /** The class of weapon this pickup will grant. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponPickup")
    TSubclassOf<AS_Weapon> WeaponClassToGrant;

protected:
    //~ Begin AS_Pickup Interface
    /** Grants the weapon or ammo to the character. */
    virtual void GivePickupTo_Implementation(AS_Character* Picker) override;
    /** Can always be picked up if WeaponClassToGrant is valid (it will either grant weapon or ammo). */
    virtual bool CanBePickedUp_Implementation(AS_Character* Picker) const override;
    //~ End AS_Pickup Interface
};