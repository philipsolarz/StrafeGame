// Source/StrafeGame/Public/Weapons/S_HitscanWeapon.h
#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_Weapon.h"
#include "S_HitscanWeapon.generated.h"

class US_HitscanWeaponDataAsset;

UCLASS(Abstract, Blueprintable)
class STRAFEGAME_API AS_HitscanWeapon : public AS_Weapon
{
    GENERATED_BODY()

public:
    AS_HitscanWeapon();

    // AS_HitscanWeapon does not override ExecutePrimaryFire_Implementation or ExecuteSecondaryFire_Implementation here.
    // Concrete derived classes (e.g., AS_ChargedShotgun) will override those and call PerformHitscanLogic.

protected:
    /**
     * Performs the core hitscan logic including tracing and processing hits.
     * @param FireStartLocation The starting point of the trace.
     * @param AimDirection The normalized direction of the aim.
     * @param EventData Optional FGameplayEventData from the ability.
     * @param PelletCount Number of traces to perform.
     * @param SpreadAngle Max spread angle in degrees.
     * @param MaxRange Maximum range of the traces.
     * @param BaseDamage Damage per hit/pellet.
     * @param DamageTypeClass DamageType to apply.
     */
    virtual void PerformHitscanLogic(
        const FVector& FireStartLocation,
        const FVector& AimDirection,
        const FGameplayEventData& EventData,
        int32 PelletCount,
        float SpreadAngle,
        float MaxRange,
        float BaseDamage,
        TSubclassOf<class UDamageType> DamageTypeClass
    );

    /**
     * Performs a single line trace for the hitscan weapon.
     * @param TraceStart The starting point of the trace.
     * @param AimDirection The initial aiming direction.
     * @param MaxRange Maximum distance for the trace.
     * @param SpreadAngle Spread angle in degrees for this trace.
     * @param OutHitResult The FHitResult of the trace.
     * @param OutSpreadAppliedDirection The actual direction of fire after spread is applied.
     * @return True if the trace hit something, false otherwise.
     */
    virtual bool PerformSingleTrace(const FVector& TraceStart, const FVector& AimDirection, float MaxRange, float SpreadAngle, FHitResult& OutHitResult, FVector& OutSpreadAppliedDirection);

    /**
     * Processes a hit from the trace. Applies damage.
     * @param HitResult The FHitResult of the trace.
     * @param ShotDirection The direction of the shot that caused the hit.
     * @param InstigatorCharacter The character who fired the weapon.
     * @param InstigatorController The controller of the instigator.
     * @param DamageToApply The amount of damage to apply for this hit.
     * @param DamageTypeClass The type of damage to apply.
     */
    virtual void ProcessHit(const FHitResult& HitResult, const FVector& ShotDirection, AS_Character* InstigatorCharacter, AController* InstigatorController, float DamageToApply, TSubclassOf<class UDamageType> DamageTypeClass);

    // Impact effects are now primarily handled by GameplayCues triggered by Abilities,
    // using tags specified in the WeaponDataAsset.
};