#pragma once

#include "CoreMinimal.h"
#include "Abilities/Weapons/S_WeaponAbility.h"
#include "S_WeaponSecondaryAbility.generated.h"

/**
 * Base GameplayAbility for a weapon's Secondary Fire action.
 */
UCLASS(Abstract)
class STRAFEGAME_API US_WeaponSecondaryAbility : public US_WeaponAbility
{
    GENERATED_BODY()

public:
    US_WeaponSecondaryAbility();

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
    /**
     * Called from ActivateAbility after checks and committing.
     * Implement secondary fire logic here.
     */
    virtual void PerformWeaponSecondaryFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

    /** Applies the ammo cost for this ability. */
    virtual void ApplyAmmoCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

    /** Applies the cooldown for this ability. */
    virtual void ApplyAbilityCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);
};