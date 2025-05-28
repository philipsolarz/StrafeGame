#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_ProjectileWeaponDataAsset.h" // Inherits from Projectile Weapon DA
#include "S_StickyGrenadeLauncherDataAsset.generated.h"

/**
 * DataAsset for the Sticky Grenade Launcher.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Sticky Grenade Launcher DataAsset"))
class STRAFEGAME_API US_StickyGrenadeLauncherDataAsset : public US_ProjectileWeaponDataAsset
{
    GENERATED_BODY()

public:
    US_StickyGrenadeLauncherDataAsset();

    /** Maximum number of active sticky grenades this player can have deployed at once. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StickyGrenadeLauncher", meta = (ClampMin = "1"))
    int32 MaxActiveProjectiles;

    // Inherited properties to be configured for Sticky Launcher:
    // - ProjectileClass (should be AS_StickyGrenadeProjectile or a BP child)
    // - LaunchSpeed, ProjectileLifeSpan
    // - PrimaryFireAbilityClass (US_StickyGrenadeLauncherPrimaryAbility_C)
    // - SecondaryFireAbilityClass (US_StickyGrenadeLauncherSecondaryAbility_C)
    // - AmmoAttribute (e.g., StickyGrenadeAmmo)
    // - MaxAmmoAttribute
    // - InitialAmmoEffect (GE to give starting sticky grenades)
    // - AmmoCostEffect_Primary (GE for firing one sticky)
    // - CooldownTag_Primary (for the 0.333s fire rate)
    // - MuzzleFlashCueTag, etc.
};