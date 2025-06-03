// Source/StrafeGame/Public/Weapons/S_WeaponDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffect.h"         
#include "GameplayAbilitySpec.h"    
#include "GameplayTagContainer.h"   
#include "GameplayEffectTypes.h"    
#include "S_WeaponDataAsset.generated.h"

// Forward Declarations
class UGameplayAbility;
class USkeletalMesh;
class UAnimMontage;
class USoundBase;
class UTexture2D;
class UNiagaraSystem;
class AS_Weapon;
class US_WeaponViewModel; // Forward declare base ViewModel

UCLASS(BlueprintType, Abstract)
class STRAFEGAME_API US_WeaponDataAsset : public UPrimaryDataAsset
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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
    FText WeaponDisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Actor")
    TSoftClassPtr<AS_Weapon> WeaponActorClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<USkeletalMesh> WeaponMeshAsset;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attachment")
    FName AttachmentSocketName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attachment")
    FName MuzzleFlashSocketName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
    float EquipTime;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
    float UnequipTime;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSoftObjectPtr<UTexture2D> CrosshairTexture;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSoftObjectPtr<UTexture2D> WeaponIcon;

    // ADDED: Specify the ViewModel class for this weapon type
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (DisplayName = "Weapon View Model Class"))
    TSubclassOf<US_WeaponViewModel> WeaponViewModelClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Mechanics", meta = (ClampMin = "0.0"))
    float MaxAimTraceRange;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
    TSubclassOf<UGameplayAbility> PrimaryFireAbilityClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
    TSubclassOf<UGameplayAbility> SecondaryFireAbilityClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
    TSubclassOf<UGameplayAbility> EquipAbilityClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Ammo")
    FGameplayAttribute AmmoAttribute;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Ammo")
    FGameplayAttribute MaxAmmoAttribute;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Ammo", meta = (DisplayName = "Initial Ammo Grant Effect"))
    TSubclassOf<UGameplayEffect> InitialAmmoEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Ammo", meta = (DisplayName = "Primary Fire Ammo Cost Effect"))
    TSubclassOf<UGameplayEffect> AmmoCostEffect_Primary;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Ammo", meta = (DisplayName = "Secondary Fire Ammo Cost Effect"))
    TSubclassOf<UGameplayEffect> AmmoCostEffect_Secondary;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Cooldowns", meta = (DisplayName = "Primary Fire Cooldown Tag"))
    FGameplayTag CooldownTag_Primary;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Cooldowns", meta = (DisplayName = "Secondary Fire Cooldown Tag"))
    FGameplayTag CooldownTag_Secondary;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues", meta = (DisplayName = "Muzzle Flash Cue"))
    FGameplayTag MuzzleFlashCueTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues", meta = (DisplayName = "Impact Cue (Generic)"))
    FGameplayTag ImpactEffectCueTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues", meta = (DisplayName = "Equip Cue"))
    FGameplayTag EquipCueTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|GameplayCues", meta = (DisplayName = "Out Of Ammo Cue"))
    FGameplayTag OutOfAmmoCueTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> FireMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> EquipMontage;

public:
    static FPrimaryAssetType GetPrimaryAssetType() { return FPrimaryAssetType(TEXT("WeaponData")); }
};