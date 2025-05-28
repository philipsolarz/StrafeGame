#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_Weapon.h" // Inherit from our base S_Weapon
#include "S_HitscanWeapon.generated.h"

// Forward declarations
class UNiagaraSystem;
class USoundBase;
struct FGameplayTag;

UCLASS(Abstract, Blueprintable) // Abstract as specific hitscan weapons (e.g. Shotgun, Rifle) will derive from this
class STRAFEGAME_API AS_HitscanWeapon : public AS_Weapon
{
    GENERATED_BODY()

public:
    AS_HitscanWeapon();

    //~ Begin AS_Weapon Interface
    /** Overrides base ExecuteFire to perform hitscan logic. */
    virtual void ExecuteFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData, float HitscanSpread, float HitscanRange, TSubclassOf<AS_Projectile> ProjectileClass) override;
    //~ End AS_Weapon Interface

protected:
    /**
     * Performs the line trace for the hitscan weapon.
     * @param TraceStart The starting point of the trace.
     * @param TraceEnd The ending point of the trace.
     * @param OutHitResult The FHitResult of the trace.
     * @param SpreadAppliedDirection The actual direction of fire after spread is applied (for debug/effects).
     * @return True if the trace hit something, false otherwise.
     */
    virtual bool PerformTrace(const FVector& TraceStart, const FVector& AimDirection, float MaxRange, float SpreadAngle, FHitResult& OutHitResult, FVector& SpreadAppliedDirection);

    /**
     * Processes a hit from the trace. Applies damage, plays impact effects.
     * Called on server.
     * @param HitResult The FHitResult of the trace.
     * @param ShotDirection The direction of the shot that caused the hit.
     * @param InstigatorCharacter The character who fired the weapon.
     * @param InstigatorController The controller of the instigator.
     */
    virtual void ProcessHit(const FHitResult& HitResult, const FVector& ShotDirection, AS_Character* InstigatorCharacter, AController* InstigatorController);

    /**
     * Plays impact effects at the hit location or trace end point.
     * Can be called on server and clients via GameplayCue or RPC.
     * For now, this will be triggered by a GameplayCue defined in WeaponDataAsset.
     * @param EndPoint The location where the impact effect should play.
     * @param HitResult Optional: provides more context about the hit surface.
     * @param bHitTarget True if the trace hit a valid target.
     */
    virtual void PlayImpactEffects(const FVector& EndPoint, const FHitResult* HitResult = nullptr, bool bHitTarget = false);

    // These properties would typically come from this weapon's specific UWeaponDataAsset,
    // but are shown here as examples of what a HitscanWeapon might need.
    // The GameplayAbility will read these from the DataAsset and pass them to ExecuteFire or use them.

    // UPROPERTY(EditDefaultsOnly, Category="Hitscan Properties")
    // float BaseDamage;

    // UPROPERTY(EditDefaultsOnly, Category="Hitscan Properties")
    // TSubclassOf<UDamageType> DamageTypeClass;

    // UPROPERTY(EditDefaultsOnly, Category="Hitscan Properties | Effects")
    // UNiagaraSystem* ImpactEffect_Default;

    // UPROPERTY(EditDefaultsOnly, Category="Hitscan Properties | Effects")
    // UNiagaraSystem* ImpactEffect_Flesh;

    // UPROPERTY(EditDefaultsOnly, Category="Hitscan Properties | Effects")
    // FGameplayTag ImpactCueTag; // GameplayCue for impacts, defined in WeaponDataAsset
};