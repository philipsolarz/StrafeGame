#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_ProjectileWeapon.h" // Inherits from our base ProjectileWeapon
#include "S_RocketLauncher.generated.h"

/**
 * Rocket Launcher weapon. Fires rocket projectiles.
 * Secondary fire (detonation) will be handled by a GameplayAbility.
 */
UCLASS(Blueprintable)
class STRAFEGAME_API AS_RocketLauncher : public AS_ProjectileWeapon
{
    GENERATED_BODY()

public:
    AS_RocketLauncher();

    // The ExecuteFire_Implementation from AS_ProjectileWeapon should be sufficient,
    // as it just needs the ProjectileClass to spawn, which the RocketLauncher's PrimaryFireAbility
    // will provide from this weapon's US_RocketLauncherDataAsset.

    // If the RocketLauncher had a unique way of launching (e.g., multiple rockets in a pattern from one fire command)
    // then ExecuteFire_Implementation would be overridden here.

    /**
     * Called by its SecondaryFire GameplayAbility (DetonateRocketsAbility) to detonate
     * the oldest active rocket projectile.
     * Server-authoritative.
     * @return True if a rocket was told to detonate, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "RocketLauncher")
    bool DetonateOldestActiveRocket();

protected:
    // Specific properties for the Rocket Launcher could go here if not in DataAsset.
    // For now, it's lean.
};