#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_ProjectileWeapon.h"
#include "S_StickyGrenadeLauncher.generated.h"

class AS_StickyGrenadeProjectile;

UCLASS(Blueprintable)
class STRAFEGAME_API AS_StickyGrenadeLauncher : public AS_ProjectileWeapon
{
    GENERATED_BODY()

public:
    AS_StickyGrenadeLauncher();

    // Max number of active sticky grenades this player can have out at once.
    // Read from DataAsset by its PrimaryFireAbility to check before firing.
    // UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StickyGrenadeLauncher")
    // int32 MaxActiveStickies; // This logic is better in the GameplayAbility using GetActiveProjectiles().Num()

    /**
     * Called by its SecondaryFire GameplayAbility to detonate the oldest active sticky grenade.
     * Server-authoritative.
     * @return True if a grenade was told to detonate, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "StickyGrenadeLauncher")
    bool DetonateOldestActiveSticky();

    // Override to manage a specific count of active stickies if needed,
    // though ActiveProjectiles in AS_ProjectileWeapon can be used.
    // The GameplayAbility will be responsible for checking count before firing.
    // virtual void RegisterProjectile(AS_Projectile* Projectile) override;

protected:
    // For Sticky Launcher, ActiveProjectiles array in AS_ProjectileWeapon will store AS_StickyGrenadeProjectile instances.
};