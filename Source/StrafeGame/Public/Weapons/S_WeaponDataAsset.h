#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffect.h"         // For TSubclassOf<UGameplayEffect>
#include "GameplayAbilitySpec.h"    // For FPrimaryAssetId
#include "GameplayTagContainer.h"   // For FGameplayTag
#include "GameplayEffectTypes.h"    // For FGameplayAttribute
#include "S_WeaponDataAsset.generated.h"

// Forward Declarations
class UGameplayAbility;
class USkeletalMesh;
class UAnimMontage;
class USoundBase;
class UTexture2D;
class UNiagaraSystem;
class AS_Weapon; // The base weapon class this DA might be for

/**
 * Base DataAsset class for defining weapon properties.
 * Contains common data applicable to all weapon types.
 */
UCLASS(BlueprintType, Abstract) // Abstract as you'll use derived versions
class STRAFEGAME_API US_WeaponDataAsset : public UPrimaryDataAsset // Inherit from UPrimaryDataAsset for Asset Manager
{
    GENERATED_BODY()

public:
    US_WeaponDataAsset();

    //~ Begin UObject Interface
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
    //~ End UObject Interface

    //~ Begin UPrimaryDataAsset interface
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
    //~ End UPrimaryDataAsset interface

    /** User-facing display name for the weapon. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
    FText WeaponDisplayName;

    /** Class of the AS_Weapon actor this DataAsset represents. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Actor")
    TSoftClassPtr<AS_Weapon> WeaponActorClass;

    // --- Visuals & Audio ---
    /** Skeletal Mesh for the weapon. Using TSoftObjectPtr for async loading. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<USkeletalMesh> WeaponMeshAsset;

    /** Socket on the character's mesh where this weapon attaches. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attachment")
    FName AttachmentSocketName;

    /** Time it takes to switch to this weapon. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
    float EquipTime;

    /** Time it takes to unequip this weapon (often same as EquipTime). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
    float UnequipTime; // Corresponds to WeaponStats.WeaponSwitchTime in old DA

    // --- UI ---
    /** Crosshair texture for this weapon. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSoftObjectPtr<UTexture2D> CrosshairTexture;

    /** Icon for UI representation (e.g., inventory, weapon selection). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSoftObjectPtr<UTexture2D> WeaponIcon;

    // --- GAS Related Properties ---

    /** Primary fire ability to grant when this weapon is equipped. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
    TSubclassOf<UGameplayAbility> PrimaryFireAbilityClass;

    /** Secondary fire ability to grant when this weapon is equipped (optional). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
    TSubclassOf<UGameplayAbility> SecondaryFireAbilityClass;

    /** Equip ability to grant when this weapon is equipped (optional, e.g., for special equip animations/effects). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
    TSubclassOf<UGameplayAbility> EquipAbilityClass;

    // --- Ammo ---
    /** The GameplayAttribute for this weapon's current ammo (e.g., GetShotgunAmmoAttribute()). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Ammo")
    FGameplayAttribute AmmoAttribute;

    /** The GameplayAttribute for this weapon's maximum ammo (e.g., GetMaxShotgunAmmoAttribute()). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Ammo")
    FGameplayAttribute MaxAmmoAttribute;

    /**
     * GameplayEffect to apply when this weapon is first added to inventory or picked up.
     * Should grant initial ammo amount and set the MaxAmmo attribute.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Ammo", meta = (DisplayName = "Initial Ammo Grant Effect"))
    TSubclassOf<UGameplayEffect> InitialAmmoEffect;

    /** Cost GameplayEffect for the primary fire. Applied by the PrimaryFireAbility. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Ammo", meta = (DisplayName = "Primary Fire Ammo Cost Effect"))
    TSubclassOf<UGameplayEffect> AmmoCostEffect_Primary;

    /** Cost GameplayEffect for the secondary fire. Applied by the SecondaryFireAbility (if one exists). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Ammo", meta = (DisplayName = "Secondary Fire Ammo Cost Effect"))
    TSubclassOf<UGameplayEffect> AmmoCostEffect_Secondary;

    // --- Cooldowns ---
    /** GameplayTag representing the cooldown for the primary fire. Used by the PrimaryFireAbility. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Cooldowns", meta = (DisplayName = "Primary Fire Cooldown Tag"))
    FGameplayTag CooldownTag_Primary;

    /** GameplayTag representing the cooldown for the secondary fire. Used by the SecondaryFireAbility. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Cooldowns", meta = (DisplayName = "Secondary Fire Cooldown Tag"))
    FGameplayTag CooldownTag_Secondary;

    // --- Effects & Sounds (via GameplayCues primarily) ---
    /** GameplayCue tag for muzzle flash effects. Triggered by fire abilities. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues", meta = (DisplayName = "Muzzle Flash Cue"))
    FGameplayTag MuzzleFlashCueTag;

    /** GameplayCue tag for weapon impact effects (generic, hitscan might have more specific). Triggered by abilities or weapon. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues", meta = (DisplayName = "Impact Cue (Generic)"))
    FGameplayTag ImpactEffectCueTag;

    /** GameplayCue tag for weapon equip effects/sounds. Triggered by EquipAbility or Inventory. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues", meta = (DisplayName = "Equip Cue"))
    FGameplayTag EquipCueTag;

    /** GameplayCue tag for out-of-ammo feedback. Triggered by fire abilities if CanActivate fails due to ammo. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues", meta = (DisplayName = "Out Of Ammo Cue"))
    FGameplayTag OutOfAmmoCueTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> FireMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> EquipMontage;

public:
    // Helper to get the PrimaryAssetType, used by Asset Manager
    static FPrimaryAssetType GetPrimaryAssetType() { return FPrimaryAssetType(TEXT("WeaponData")); }
};