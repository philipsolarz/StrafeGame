#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_WeaponDataAsset.h" // Inherit from base
#include "S_HitscanWeaponDataAsset.generated.h"

/**
 * DataAsset for Hitscan-specific weapon properties.
 */
UCLASS(BlueprintType)
class STRAFEGAME_API US_HitscanWeaponDataAsset : public US_WeaponDataAsset
{
    GENERATED_BODY()

public:
    US_HitscanWeaponDataAsset();

    /** Base damage per hit/pellet for this hitscan weapon. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan", meta = (ClampMin = "0.0"))
    float BaseDamage;

    /** Maximum range of the hitscan traces. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan", meta = (ClampMin = "0.0"))
    float MaxRange;

    /** Max spread angle in degrees from the center. 0 for perfect accuracy. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan", meta = (ClampMin = "0.0", ClampMax = "45.0"))
    float SpreadAngle;

    /** Number of pellets fired per shot (e.g., 1 for rifle, 8-12 for shotgun). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan", meta = (ClampMin = "1"))
    int32 PelletCount;

    /** DamageType class to apply on hit. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan")
    TSubclassOf<class UDamageType> DamageTypeClass;

    /** GameplayCue tag for specific hitscan impact effects (e.g., a more detailed tracer or surface effect).
     * This can override or supplement the generic ImpactEffectCueTag from the base US_WeaponDataAsset. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues", meta = (DisplayName = "Hitscan Impact Cue"))
    FGameplayTag HitscanImpactCueTag;

    // You could add more specific hitscan properties here if needed,
    // e.g., critical hit multiplier, falloff parameters, etc.
};