#pragma once

#include "CoreMinimal.h"
#include "Abilities/Weapons/S_WeaponPrimaryAbility.h"
#include "S_ChargedShotgunPrimaryAbility.generated.h"

class UAbilityTask_WaitDelay;
class UAbilityTask_WaitInputRelease;
class AS_ChargedShotgun; // Forward declare
class US_ChargedShotgunDataAsset; // Forward declare

UCLASS()
class STRAFEGAME_API US_ChargedShotgunPrimaryAbility : public US_WeaponPrimaryAbility
{
    GENERATED_BODY()

public:
    US_ChargedShotgunPrimaryAbility();

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override; // Called when input is released if ability is active

protected:
    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitInputRelease> WaitInputReleaseTask;

    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitDelay> ChargeTimerTask;

    bool bIsCharging;
    bool bFiredDuringChargeLoop; // To prevent immediate re-fire if input held after auto-fire

    // GameplayTags used by this ability
    FGameplayTag ChargeInProgressTag;
    FGameplayTag EarlyReleaseCooldownTag; // Tag applied by the early release cooldown GE

    /** Starts or restarts the charging process. */
    void StartCharge();

    /** Called when the charge timer completes. */
    UFUNCTION()
    void OnChargeComplete();

    /** Handles the actual firing logic. */
    void DoFire();

    /** Applies the early release penalty cooldown. */
    void ApplyEarlyReleasePenalty();

    /** Resets ability state flags. */
    void ResetAbilityState();

    // Helper to get typed weapon and data asset
    AS_ChargedShotgun* GetChargedShotgun() const;
    const US_ChargedShotgunDataAsset* GetChargedShotgunData() const;
};