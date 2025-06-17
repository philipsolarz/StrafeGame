#pragma once

#include "CoreMinimal.h"
#include "Items/Pickups/S_Pickup.h"
#include "S_ArmorPickup.generated.h"

class UGameplayEffect;

/**
 * A pickup that grants armor to a character.
 */
UCLASS(Blueprintable)
class STRAFEGAME_API AS_ArmorPickup : public AS_Pickup
{
	GENERATED_BODY()

public:
	AS_ArmorPickup();

	/**
	 * The GameplayEffect to apply to grant armor.
	 * This GE should target the Armor attribute with an Additive modifier.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ArmorPickup")
	TSubclassOf<UGameplayEffect> ArmorGrantEffect;

protected:
	virtual void GivePickupTo_Implementation(AS_Character* Picker) override;
	virtual bool CanBePickedUp_Implementation(AS_Character* Picker) const override;
};