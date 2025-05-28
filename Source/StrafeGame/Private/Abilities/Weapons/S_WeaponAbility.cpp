#include "Abilities/Weapons/S_WeaponAbility.h"
#include "Player/S_Character.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"

US_WeaponAbility::US_WeaponAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    AbilityInputID = -1;
    bActivateOnEquip = false;
    bCancelOnUnequip = true;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer; // Default, adjust if needed
}

AS_Character* US_WeaponAbility::GetOwningSCharacter() const
{
    return Cast<AS_Character>(GetAvatarActorFromActorInfo());
}

AS_Weapon* US_WeaponAbility::GetEquippedWeapon() const
{
    AS_Character* Char = GetOwningSCharacter();
    return Char ? Char->GetCurrentWeapon() : nullptr;
}

const US_WeaponDataAsset* US_WeaponAbility::GetEquippedWeaponData() const
{
    AS_Weapon* Weapon = GetEquippedWeapon();
    return Weapon ? Weapon->GetWeaponData() : nullptr;
}

UAbilityTask_PlayMontageAndWait* US_WeaponAbility::PlayWeaponMontage(
    TSoftObjectPtr<UAnimMontage> MontageAsset,
    float Rate,
    FName StartSectionName,
    bool bStopWhenAbilityEnds)
{
    AS_Character* SChar = GetOwningSCharacter();
    if (!SChar || !MontageAsset.IsValid())
    {
        return nullptr;
    }

    UAnimMontage* MontageToPlay = MontageAsset.LoadSynchronous(); // Load if not already loaded

    if (MontageToPlay)
    {
        UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,
            MontageToPlay,
            Rate,
            StartSectionName,
            bStopWhenAbilityEnds
        );
        return PlayMontageTask;
    }
    return nullptr;
}

const FGameplayAbilityActorInfo* US_WeaponAbility::GetAbilityActorInfo() const
{
    return CurrentActorInfo;
}