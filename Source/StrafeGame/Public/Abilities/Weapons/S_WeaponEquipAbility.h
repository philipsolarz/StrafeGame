#pragma once

#include "CoreMinimal.h"
#include "Abilities/Weapons/S_WeaponAbility.h"
#include "S_WeaponEquipAbility.generated.h"

/**
 * GameplayAbility for handling weapon equip actions (animations, sounds, initial effects).
 * Typically set to bActivateOnEquip = true in the WeaponDataAsset.
 */
UCLASS()
class STRAFEGAME_API US_WeaponEquipAbility : public US_WeaponAbility
{
    GENERATED_BODY()

public:
    US_WeaponEquipAbility();

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const F* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> EquipMontageTask;

    UFUNCTION()
    void OnMontageCompleted();
    UFUNCTION()
    void OnMontageInterruptedOrCancelled();
};