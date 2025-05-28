#include "Abilities/Weapons/S_WeaponEquipAbility.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h" // For UAnimMontage

US_WeaponEquipAbility::US_WeaponEquipAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    bActivateOnEquip = true;
    bCancelOnUnequip = true;
    AbilityInputID = -1;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

bool US_WeaponEquipAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }
    const AS_Weapon* EquippedWeapon = GetEquippedWeapon();
    if (!EquippedWeapon || !EquippedWeapon->GetWeaponData()) return false;

    // Check if weapon is already equipped or is currently unequipping
    if (EquippedWeapon->GetCurrentWeaponState() == EWeaponState::Equipped || EquippedWeapon->GetCurrentWeaponState() == EWeaponState::Unequipping)
    {
        return false;
    }

    return true;
}

void US_WeaponEquipAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AS_Character* Character = GetOwningSCharacter();
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();

    if (!Character || !WeaponData)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Use the single montage from WeaponData (e.g., WeaponData->EquipMontage)
    // Your US_WeaponDataAsset should now have a TSoftObjectPtr<UAnimMontage> EquipMontage;
    EquipMontageTask = PlayWeaponMontage(WeaponData->EquipMontage); // Assuming WeaponData->EquipMontage exists
    if (EquipMontageTask)
    {
        EquipMontageTask->OnCompleted.AddDynamic(this, &US_WeaponEquipAbility::OnMontageCompleted);
        EquipMontageTask->OnInterrupted.AddDynamic(this, &US_WeaponEquipAbility::OnMontageInterruptedOrCancelled);
        EquipMontageTask->OnCancelled.AddDynamic(this, &US_WeaponEquipAbility::OnMontageInterruptedOrCancelled);
        EquipMontageTask->ReadyForActivation();
    }
    else
    {
        // No montage, or failed to play.
        // Play Equip Cue if it exists.
        if (WeaponData->EquipCueTag.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
        {
            FGameplayCueParameters CueParams;
            // Populate CueParams if needed, e.g. for location/attachment
            // CueParams.Location = Character->GetActorLocation();
            CueParams.Instigator = Character;
            CueParams.EffectCauser = GetEquippedWeapon();
            ActorInfo->AbilitySystemComponent->ExecuteGameplayCue(WeaponData->EquipCueTag, CueParams);
        }
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false); // End successfully if no montage needed
    }
}

void US_WeaponEquipAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (EquipMontageTask && EquipMontageTask->IsActive())
    {
        EquipMontageTask->EndTask();
    }
    EquipMontageTask = nullptr;
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void US_WeaponEquipAbility::OnMontageCompleted()
{
    // Play Equip Cue if it exists, after montage.
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();
    AS_Character* Character = GetOwningSCharacter();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    if (WeaponData && WeaponData->EquipCueTag.IsValid() && Character && ASC)
    {
        FGameplayCueParameters CueParams;
        CueParams.Instigator = Character;
        CueParams.EffectCauser = GetEquippedWeapon();
        ASC->ExecuteGameplayCue(WeaponData->EquipCueTag, CueParams);
    }
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void US_WeaponEquipAbility::OnMontageInterruptedOrCancelled()
{
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}