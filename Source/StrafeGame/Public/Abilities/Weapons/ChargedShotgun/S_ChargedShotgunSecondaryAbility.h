#pragma once

#include "CoreMinimal.h"
#include "Abilities/Weapons/S_WeaponSecondaryAbility.h"
#include "S_ChargedShotgunSecondaryAbility.generated.h"

class UAbilityTask_WaitDelay;
class UAbilityTask_WaitInputRelease;
class AS_ChargedShotgun;
class US_ChargedShotgunDataAsset;

UCLASS()
class STRAFEGAME_API US_ChargedShotgunSecondaryAbility : public US_WeaponSecondaryAbility
{
    GENERATED_BODY()

public:
    US_ChargedShotgunSecondaryAbility();

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
    virtual void PerformWeaponSecondaryFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitInputRelease> WaitInputReleaseTask;

    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitDelay> ChargeTimerTask;

    bool bIsCharging;
    bool bOverchargedShotStored;
    bool bInputReleasedDuringChargeAttempt; // Flag to check if input was let go while trying to charge
    float ChargeStartTime; // Track when charging started
    float ChargeDuration; // Store the charge duration

    // GameplayTags
    FGameplayTag ChargeInProgressTag;
    FGameplayTag OverchargedStateTag;
    // Lockout tag comes from DataAsset

    // Timer handle for progress updates
    FTimerHandle ChargeProgressTimerHandle;

    void StartSecondaryCharge();

    UFUNCTION()
    void OnSecondaryChargeComplete();

    void UpdateChargeProgress();
    void AttemptFireOverchargedShot();
    void ApplyWeaponLockoutCooldown();
    void ResetAbilityState();

    AS_ChargedShotgun* GetChargedShotgun() const;
    const US_ChargedShotgunDataAsset* GetChargedShotgunData() const;
};