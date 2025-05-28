#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_HitscanWeapon.h" // Inherits from our base HitscanWeapon
#include "S_ChargedShotgun.generated.h"

/**
 * A hitscan weapon that can be charged for primary fire,
 * and over-charged for a powerful secondary shot.
 * Most of the charging and firing logic is handled by its specific GameplayAbilities.
 */
UCLASS(Blueprintable) // Blueprintable to create BP_S_ChargedShotgun
class STRAFEGAME_API AS_ChargedShotgun : public AS_HitscanWeapon
{
    GENERATED_BODY()

public:
    AS_ChargedShotgun();

    // AS_ChargedShotgun might not need to override ExecuteFire_Implementation
    // if AS_HitscanWeapon's implementation is generic enough to take pellet count,
    // spread, etc., as parameters, which the specific ChargedShotgun GameplayAbilities
    // will provide from this weapon's US_ChargedShotgunDataAsset.

    // If there were unique aspects to *how* the shotgun performs its hitscan beyond
    // what the base AS_HitscanWeapon::ExecuteFire_Implementation handles with parameters,
    // you would override it here. For now, let's assume the base is sufficient.
    // virtual void ExecuteFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData* EventData, float HitscanSpread, float HitscanRange, TSubclassOf<AS_Projectile> ProjectileClass) override;


    // --- Blueprint Implementable Events for Charging Effects ---
    // These can be called by the weapon's GameplayAbilities to trigger visual/audio feedback.

    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Primary Charge Start"))
    void K2_OnPrimaryChargeStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Primary Charge Complete"))
    void K2_OnPrimaryChargeComplete(); // Called when auto-fire happens or charge is ready

    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Primary Charge Cancelled"))
    void K2_OnPrimaryChargeCancelled();


    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Secondary Charge Start"))
    void K2_OnSecondaryChargeStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Secondary Charge Held (Fully Charged)"))
    void K2_OnSecondaryChargeHeld(); // Called when secondary is fully charged and being held

    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Secondary Charge Released (Fired)"))
    void K2_OnSecondaryChargeReleased();

    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Secondary Charge Cancelled"))
    void K2_OnSecondaryChargeCancelled(); // If applicable (though you said no penalty)

protected:
    // Specific properties for the Charged Shotgun that might not fit neatly into the DataAsset
    // or are intrinsic to its C++ behavior could go here.
    // For example, if it had a visible charging meter managed by C++.
    // However, most unique parameters (charge times, specific pellet counts for primary vs secondary)
    // will be defined in a US_ChargedShotgunDataAsset (derived from US_HitscanWeaponDataAsset)
    // and used by the specific US_ChargedShotgunPrimaryAbility and US_ChargedShotgunSecondaryAbility.

    // For now, this class can be quite lean, relying on its DataAsset and Abilities.
};