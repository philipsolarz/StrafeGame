#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_HitscanWeaponDataAsset.h" // Inherits from Hitscan Weapon DA
#include "GameplayTagContainer.h" // For FGameplayTag
#include "GameplayEffect.h"       // For TSubclassOf<UGameplayEffect>
#include "S_ChargedShotgunDataAsset.generated.h"

class UGameplayEffect;
class USoundBase;

/**
 * DataAsset for the Charged Shotgun, adding specific parameters for its unique mechanics
 * on top of the standard HitscanWeaponDataAsset properties.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Charged Shotgun DataAsset"))
class STRAFEGAME_API US_ChargedShotgunDataAsset : public US_HitscanWeaponDataAsset
{
    GENERATED_BODY()

public:
    US_ChargedShotgunDataAsset();

    // --- Primary Fire: Charge & Auto-Fire ---

    /** Time in seconds it takes for the primary fire to fully charge. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedShotgun|PrimaryFire", meta = (ClampMin = "0.1"))
    float PrimaryChargeTime;

    /**
     * GameplayEffect to apply if the primary fire input is released *before* full charge.
     * This effect should typically apply a short cooldown using the CooldownTag_Primary.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedShotgun|PrimaryFire", meta = (DisplayName = "Primary Early Release Cooldown Effect"))
    TSubclassOf<UGameplayEffect> PrimaryEarlyReleaseCooldownEffect;

    /** Specific pellet count for primary fire, overrides PelletCount from US_HitscanWeaponDataAsset if needed, or use that one. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedShotgun|PrimaryFire", meta = (ClampMin = "1"))
    int32 PrimaryFirePelletCount;

    /** Specific spread angle for primary fire. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedShotgun|PrimaryFire", meta = (ClampMin = "0.0", ClampMax = "45.0"))
    float PrimaryFireSpreadAngle;

    /** Specific max range for primary fire hitscan. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedShotgun|PrimaryFire", meta = (ClampMin = "0.0"))
    float PrimaryFireHitscanRange;

    /** Specific base damage per pellet for primary fire. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedShotgun|PrimaryFire", meta = (ClampMin = "0.0"))
    float PrimaryFireDamagePerPellet;


    // --- Secondary Fire: Charge & Hold, Fire on Release ---

    /** Time in seconds it takes for the secondary fire to fully charge. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedShotgun|SecondaryFire", meta = (ClampMin = "0.1"))
    float SecondaryChargeTime;

    /**
     * GameplayEffect to apply after a successful secondary fire.
     * This effect should apply a cooldown tag that blocks both primary and secondary fire abilities.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedShotgun|SecondaryFire", meta = (DisplayName = "Secondary Fire Lockout Effect"))
    TSubclassOf<UGameplayEffect> SecondaryFireLockoutEffect;

    /**
     * The GameplayTag that the SecondaryFireLockoutEffect applies.
     * Both primary and secondary fire abilities for the charged shotgun will check if this tag is present
     * on the owning ASC and will fail CanActivateAbility if it is.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedShotgun|SecondaryFire", meta = (DisplayName = "Secondary Fire Lockout Tag"))
    FGameplayTag SecondaryFireLockoutTag;

    /** Specific pellet count for secondary (overcharged) fire. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedShotgun|SecondaryFire", meta = (ClampMin = "1"))
    int32 SecondaryFirePelletCount;

    /** Specific spread angle for secondary (overcharged) fire. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedShotgun|SecondaryFire", meta = (ClampMin = "0.0", ClampMax = "45.0"))
    float SecondaryFireSpreadAngle;

    /** Specific max range for secondary (overcharged) fire hitscan. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedShotgun|SecondaryFire", meta = (ClampMin = "0.0"))
    float SecondaryFireHitscanRange;

    /** Specific base damage per pellet for secondary (overcharged) fire. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedShotgun|SecondaryFire", meta = (ClampMin = "0.0"))
    float SecondaryFireDamagePerPellet;


    // --- Gameplay Cues for Charging (Specific to Charged Shotgun) ---

    /** GameplayCue tag for when primary charge begins. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues|ChargedShotgun", meta = (DisplayName = "Primary Charge Start Cue"))
    FGameplayTag PrimaryChargeStartCue;

    /** GameplayCue tag for loopable primary charging effect (optional). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues|ChargedShotgun", meta = (DisplayName = "Primary Charge Loop Cue"))
    FGameplayTag PrimaryChargeLoopCue;

    /** GameplayCue tag for when primary charge completes / fires. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues|ChargedShotgun", meta = (DisplayName = "Primary Charge Complete/Fire Cue"))
    FGameplayTag PrimaryChargeCompleteCue;

    /** GameplayCue tag for when secondary charge begins. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues|ChargedShotgun", meta = (DisplayName = "Secondary Charge Start Cue"))
    FGameplayTag SecondaryChargeStartCue;

    /** GameplayCue tag for loopable secondary charging effect (optional). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues|ChargedShotgun", meta = (DisplayName = "Secondary Charge Loop Cue"))
    FGameplayTag SecondaryChargeLoopCue;

    /** GameplayCue tag for when secondary charge is fully charged and being held. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues|ChargedShotgun", meta = (DisplayName = "Secondary Charge Held (Full) Cue"))
    FGameplayTag SecondaryChargeHeldCue;

    /** GameplayCue tag for when secondary overcharged shot is fired. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues|ChargedShotgun", meta = (DisplayName = "Secondary Overcharged Fire Cue"))
    FGameplayTag SecondaryOverchargedFireCue;


    // --- Sounds (Specific to Charged Shotgun, if not handled by Cues) ---

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|ChargedShotgun")
    TSoftObjectPtr<USoundBase> PrimaryChargeSound; // Sound for primary charge start/loop

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|ChargedShotgun")
    TSoftObjectPtr<USoundBase> PrimaryFireChargedSound; // Sound for when fully charged primary fires

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|ChargedShotgun")
    TSoftObjectPtr<USoundBase> SecondaryChargeSound; // Sound for secondary charge start/loop

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|ChargedShotgun")
    TSoftObjectPtr<USoundBase> SecondaryChargeHeldSound; // Sound for when secondary is fully charged and held

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|ChargedShotgun")
    TSoftObjectPtr<USoundBase> SecondaryFireOverchargedSound; // Sound for when overcharged secondary fires
};