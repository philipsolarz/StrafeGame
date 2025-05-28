#pragma once

#include "CoreMinimal.h"
#include "Abilities/Weapons/S_WeaponAbility.h"
#include "S_WeaponPrimaryAbility.generated.h"

/**
 * Base GameplayAbility for a weapon's Primary Fire action.
 * Derived classes will implement specific fire logic (e.g., hitscan, projectile spawn).
 */
UCLASS(Abstract) // Abstract as specific weapon primary fires will derive from this
class STRAFEGAME_API US_WeaponPrimaryAbility : public US_WeaponAbility
{
    GENERATED_BODY()

public:
    US_WeaponPrimaryAbility();

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
    /**
     * Called from ActivateAbility after checks and committing.
     * This is where derived classes should implement their core fire logic.
     * (e.g., call weapon's ExecuteFire, play montages, trigger cues).
     */
    virtual void PerformWeaponFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

    /** Applies the ammo cost for this ability. */
    virtual void ApplyAmmoCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

    /** Applies the cooldown for this ability. */
    virtual void ApplyAbilityCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

    // Example Task property, if you use montages often for firing
    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> FireMontageTask;

    // Montage callbacks
    UFUNCTION()
    virtual void OnFireMontageCompleted();
    UFUNCTION()
    virtual void OnFireMontageInterruptedOrCancelled();
};