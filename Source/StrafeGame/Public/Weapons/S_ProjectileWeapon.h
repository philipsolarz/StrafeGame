// Source/StrafeGame/Public/Weapons/S_ProjectileWeapon.h
#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_Weapon.h"
#include "S_ProjectileWeapon.generated.h"

class AS_Projectile;
class US_ProjectileWeaponDataAsset;

UCLASS(Abstract, Blueprintable)
class STRAFEGAME_API AS_ProjectileWeapon : public AS_Weapon
{
    GENERATED_BODY()

public:
    AS_ProjectileWeapon();

    // AS_ProjectileWeapon does not override ExecutePrimaryFire_Implementation or ExecuteSecondaryFire_Implementation here.
    // Concrete derived classes (e.g., AS_RocketLauncher) will override those and call PerformProjectileSpawnLogic.

    // --- Projectile Management (Server-Side) ---
    virtual void RegisterProjectile(AS_Projectile* Projectile);
    virtual void UnregisterProjectile(AS_Projectile* Projectile);
    const TArray<TObjectPtr<AS_Projectile>>& GetActiveProjectiles() const { return ActiveProjectiles; }

protected:
    /**
     * Spawns and initializes a projectile.
     * @param FireStartLocation The starting point for the projectile.
     * @param FireDirection The normalized direction of the fire.
     * @param EventData Optional FGameplayEventData from the ability.
     * @param ProjectileClass The class of projectile to spawn.
     * @param LaunchSpeed The initial speed of the projectile.
     * @param ProjectileLifeSpan The lifespan of the projectile (0 for indefinite or projectile's default).
     * @return The spawned projectile, or nullptr if failed.
     */
    virtual AS_Projectile* PerformProjectileSpawnLogic(
        const FVector& FireStartLocation,
        const FVector& FireDirection,
        const FGameplayEventData& EventData,
        TSubclassOf<AS_Projectile> ProjectileClass,
        float LaunchSpeed,
        float ProjectileLifeSpan
    );

    UFUNCTION() // Needs to be UFUNCTION to bind to delegate
        virtual void OnProjectileDestroyed(AActor* DestroyedActor);

    UPROPERTY(Transient) // Server-side list of active projectiles, not replicated directly
        TArray<TObjectPtr<AS_Projectile>> ActiveProjectiles;
};