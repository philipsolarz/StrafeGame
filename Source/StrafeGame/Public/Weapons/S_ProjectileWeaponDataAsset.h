#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_WeaponDataAsset.h" // Inherit from base
#include "Weapons/S_Projectile.h"
#include "S_ProjectileWeaponDataAsset.generated.h"

// Forward Declarations
//class AS_Projectile; // Base class for projectiles

/**
 * DataAsset for Projectile-specific weapon properties.
 */
UCLASS(BlueprintType)
class STRAFEGAME_API US_ProjectileWeaponDataAsset : public US_WeaponDataAsset
{
    GENERATED_BODY()

public:
    US_ProjectileWeaponDataAsset();

    /** The class of projectile this weapon fires. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    TSubclassOf<AS_Projectile> ProjectileClass;

    /** Initial speed of the projectile when launched. If 0, projectile's internal default is used. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "0.0"))
    float LaunchSpeed;

    /** Time before a projectile self-destructs. If 0, uses projectile's internal default. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "0.0"))
    float ProjectileLifeSpan;

    /** GameplayCue tag for projectile explosion effects (if applicable). Triggered by the projectile itself. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues", meta = (DisplayName = "Projectile Explosion Cue"))
    FGameplayTag ProjectileExplosionCueTag;

    // You could add more specific projectile weapon properties here,
    // e.g., number of projectiles per shot (if it's a burst fire of projectiles), spread for projectiles, etc.
};