#include "Abilities/Weapons/S_WeaponEquipAbility.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h" 

US_WeaponEquipAbility::US_WeaponEquipAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    bActivateOnEquip = true;
    bCancelOnUnequip = true; // Important for equip abilities
    AbilityInputID = -1; // Equip abilities are not typically manually triggered by input ID
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

// Corrected signature to match UGameplayAbility
bool US_WeaponEquipAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }
    const AS_Weapon* EquippedWeapon = GetEquippedWeapon(); // GetEquippedWeapon is from US_WeaponAbility (base)
    if (!EquippedWeapon || !EquippedWeapon->GetWeaponData()) // CORRECTED: added braces and return
    {
        return false;
    }

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

    EquipMontageTask = PlayWeaponMontage(WeaponData->EquipMontage);
    if (EquipMontageTask)
    {
        EquipMontageTask->OnCompleted.AddDynamic(this, &US_WeaponEquipAbility::OnMontageCompleted);
        EquipMontageTask->OnInterrupted.AddDynamic(this, &US_WeaponEquipAbility::OnMontageInterruptedOrCancelled);
        EquipMontageTask->OnCancelled.AddDynamic(this, &US_WeaponEquipAbility::OnMontageInterruptedOrCancelled);
        EquipMontageTask->ReadyForActivation();
    }
    else
    {
        if (WeaponData->EquipCueTag.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
        {
            FGameplayCueParameters CueParams;
            CueParams.Instigator = Character;
            CueParams.EffectCauser = GetEquippedWeapon();
            ActorInfo->AbilitySystemComponent->ExecuteGameplayCue(WeaponData->EquipCueTag, CueParams);
        }
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
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