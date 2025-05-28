#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_Weapon.h" // Inherit from our base S_Weapon
#include "S_ProjectileWeapon.generated.h"

// Forward Declarations
class AS_Projectile; // Base class for all projectiles

UCLASS(Abstract, Blueprintable) // Abstract as specific projectile weapons (e.g. RocketLauncher) will derive
class STRAFEGAME_API AS_ProjectileWeapon : public AS_Weapon
{
    GENERATED_BODY()

public:
    AS_ProjectileWeapon();

    //~ Begin AS_Weapon Interface
    /** Overrides base ExecuteFire to spawn a projectile. */
    virtual void ExecuteFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData* EventData, float HitscanSpread, float HitscanRange, TSubclassOf<AS_Projectile> ProjectileClass) override;
    //~ End AS_Weapon Interface

    // --- Projectile Management (Server-Side) ---
    // These were in the base AS_Weapon before, now specific to ProjectileWeapon
    virtual void RegisterProjectile(AS_Projectile* Projectile);
    virtual void UnregisterProjectile(AS_Projectile* Projectile);
    const TArray<TObjectPtr<AS_Projectile>>& GetActiveProjectiles() const { return ActiveProjectiles; }

protected:
    /**
     * Spawns the projectile.
     * @param ProjectileClass The class of projectile to spawn (passed from GameplayAbility, read from WeaponDataAsset).
     * @param SpawnLocation Location to spawn the projectile.
     * @param SpawnRotation Rotation for the spawned projectile.
     * @param InstigatorCharacter The character firing the weapon.
     * @param InstigatorController The controller of the instigator.
     * @return The spawned projectile, or nullptr if failed.
     */
    virtual AS_Projectile* SpawnProjectile(TSubclassOf<AS_Projectile> ProjectileClass, const FVector& SpawnLocation, const FRotator& SpawnRotation, AS_Character* InstigatorCharacter, AController* InstigatorController);

    UFUNCTION() // Needs to be UFUNCTION to bind to delegate
        virtual void OnProjectileDestroyed(AActor* DestroyedActor);

    UPROPERTY() // Server-side list of active projectiles, not replicated directly
        TArray<TObjectPtr<AS_Projectile>> ActiveProjectiles;

    // Default projectile class for this weapon.
    // This should ideally be specified in this weapon's US_WeaponDataAsset and passed to ExecuteFire by the GA.
    // UPROPERTY(EditDefaultsOnly, Category="ProjectileWeapon")
    // TSubclassOf<AS_Projectile> DefaultProjectileClass;
};