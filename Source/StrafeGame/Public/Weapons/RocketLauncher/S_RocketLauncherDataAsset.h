#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_ProjectileWeaponDataAsset.h" // Inherits from Projectile Weapon DA
#include "S_RocketLauncherDataAsset.generated.h"

/**
 * DataAsset for the Rocket Launcher.
 * Configures properties specific to the Rocket Launcher, inheriting from ProjectileWeaponDataAsset.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Rocket Launcher DataAsset"))
class STRAFEGAME_API US_RocketLauncherDataAsset : public US_ProjectileWeaponDataAsset
{
    GENERATED_BODY()

public:
    US_RocketLauncherDataAsset();

    // Most properties specific to the Rocket Launcher's projectile (AS_RocketProjectile)
    // will be set directly on the AS_RocketProjectile Blueprint or its C++ defaults.
    // This DataAsset primarily serves to:
    // 1. Assign the correct AS_RocketProjectile class to the inherited ProjectileClass property.
    // 2. Configure abilities (PrimaryFireAbilityClass = US_RocketLauncherPrimaryAbility_C,
    //    SecondaryFireAbilityClass = US_RocketLauncherSecondaryAbility_C).
    // 3. Configure ammo (RocketAmmo attribute, cost GEs, initial ammo GE).
    // 4. Configure cooldowns (Primary fire rate of 1s).
    // 5. Configure GameplayCues (Muzzle flash, explosion cue if not on projectile itself).
    // 6. Configure visual/audio assets (Mesh, sounds, icons etc. from US_WeaponDataAsset).

    // Example: If you wanted a specific stat for all Rocket Launchers not covered by base classes:
    // UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RocketLauncher")
    // float SomeRocketLauncherSpecificStat;
};