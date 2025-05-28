// Source/StrafeGame/Private/Abilities/Weapons/S_WeaponAbility.cpp
#include "Abilities/Weapons/S_WeaponAbility.h"
#include "Player/S_Character.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"

US_WeaponAbility::US_WeaponAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    AbilityInputID = -1;
    bActivateOnEquip = false;
    bCancelOnUnequip = true;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
    UE_LOG(LogTemp, Log, TEXT("US_WeaponAbility::US_WeaponAbility: Constructor for %s. InputID: %d"), *GetNameSafe(this), AbilityInputID);
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
    UE_LOG(LogTemp, Log, TEXT("US_WeaponAbility::PlayWeaponMontage: %s - Montage: %s, Character: %s"), *GetNameSafe(this), *MontageAsset.ToString(), *GetNameSafe(SChar));
    if (!SChar || !MontageAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("US_WeaponAbility::PlayWeaponMontage: %s - Character or MontageAsset invalid."), *GetNameSafe(this));
        return nullptr;
    }

    UAnimMontage* MontageToPlay = MontageAsset.LoadSynchronous();

    if (MontageToPlay)
    {
        UE_LOG(LogTemp, Log, TEXT("US_WeaponAbility::PlayWeaponMontage: %s - Playing Montage %s"), *GetNameSafe(this), *MontageToPlay->GetName());
        UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,
            MontageToPlay,
            Rate,
            StartSectionName,
            bStopWhenAbilityEnds,
            1.0f
        );
        return PlayMontageTask;
    }
    UE_LOG(LogTemp, Warning, TEXT("US_WeaponAbility::PlayWeaponMontage: %s - Failed to load montage %s."), *GetNameSafe(this), *MontageAsset.ToString());
    return nullptr;
}

FGameplayAbilityActorInfo US_WeaponAbility::GetActorInfoForBlueprint() const
{
    const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
    if (Info)
    {
        return *Info;
    }
    return FGameplayAbilityActorInfo();
}

void US_WeaponAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponAbility::ActivateAbility: %s - Handle: %s, Actor: %s"), *GetNameSafe(this), *Handle.ToString(), ActorInfo ? *GetNameSafe(ActorInfo->AvatarActor.Get()) : TEXT("UnknownActor"));
    CurrentEventData = TriggerEventData;
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}