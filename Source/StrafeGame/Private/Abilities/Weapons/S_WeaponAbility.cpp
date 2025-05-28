#include "Abilities/Weapons/S_WeaponAbility.h"
#include "Player/S_Character.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
// #include "GameFramework/Character.h" // Already have S_Character

US_WeaponAbility::US_WeaponAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    AbilityInputID = -1; // Default, to be overridden
    bActivateOnEquip = false;
    bCancelOnUnequip = true;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
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
    if (!SChar || !MontageAsset.IsValid()) // Use IsValid() for TSoftObjectPtr
    {
        return nullptr;
    }

    UAnimMontage* MontageToPlay = MontageAsset.LoadSynchronous();

    if (MontageToPlay)
    {
        UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            NAME_None, // InstanceName
            MontageToPlay,
            Rate,
            StartSectionName,
            bStopWhenAbilityEnds,
            1.0f // AnimRootMotionTranslationScale
        );
        return PlayMontageTask;
    }
    return nullptr;
}

// MODIFIED Implementation
FGameplayAbilityActorInfo US_WeaponAbility::GetActorInfoForBlueprint() const
{
    const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
    if (Info)
    {
        return *Info; // Return a copy
    }
    return FGameplayAbilityActorInfo(); // Return default-constructed if null
}