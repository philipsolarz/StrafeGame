#pragma once

#include "CoreMinimal.h"
#include "Items/Pickups/S_Pickup.h"
#include "S_HealthPickup.generated.h"

class UGameplayEffect;

/**
 * A pickup that grants health to a character.
 */
UCLASS(Blueprintable)
class STRAFEGAME_API AS_HealthPickup : public AS_Pickup
{
	GENERATED_BODY()

public:
	AS_HealthPickup();

	/**
	 * The GameplayEffect to apply to grant health.
	 * This GE should target the Health attribute with an Additive modifier.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HealthPickup")
	TSubclassOf<UGameplayEffect> HealthGrantEffect;

protected:
	virtual void GivePickupTo_Implementation(AS_Character* Picker) override;
	virtual bool CanBePickedUp_Implementation(AS_Character* Picker) const override;
};