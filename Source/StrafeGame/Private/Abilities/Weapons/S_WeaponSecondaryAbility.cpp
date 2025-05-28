#include "Abilities/Weapons/S_WeaponSecondaryAbility.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Player/S_Character.h"
#include "Player/Attributes/S_AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Controller.h"


US_WeaponSecondaryAbility::US_WeaponSecondaryAbility()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    AbilityInputID = 1; // Conventionally, 1 for secondary fire.
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Action.SecondaryFire")));
}

bool US_WeaponSecondaryAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }
    const AS_Character* Character = GetOwningSCharacter();
    const AS_Weapon* EquippedWeapon = GetEquippedWeapon();
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();
    UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

    if (!Character || !EquippedWeapon || !WeaponData || !ASC)
    {
        return false;
    }
    if (EquippedWeapon->GetCurrentWeaponState() != EWeaponState::Equipped)
    {
        return false;
    }
    if (!WeaponData->SecondaryFireAbilityClass) // Check if weapon actually has a secondary fire defined
    {
        return false;
    }
    if (WeaponData->AmmoAttribute.IsValid() && WeaponData->AmmoCostEffect_Secondary) // Check for secondary ammo cost
    {
        if (ASC->GetNumericAttribute(WeaponData->AmmoAttribute) <= 0) // Or specific cost amount check
        {
            if (WeaponData->OutOfAmmoCueTag.IsValid())
            {
                ASC->ExecuteGameplayCue(WeaponData->OutOfAmmoCueTag, ASC->MakeEffectContext());
            }
            return false;
        }
    }
    return true;
}

void US_WeaponSecondaryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    PerformWeaponSecondaryFire(Handle, ActorInfo, ActivationInfo);
}

void US_WeaponSecondaryAbility::PerformWeaponSecondaryFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    // Base implementation. Derived classes will override.
    // This might involve calling a different function on AS_Weapon, or specific tasks.
    // For instance, a rocket launcher's secondary might detonate existing rockets.
    // A charged shotgun's secondary is a different fire mode.

    // Example: Generic secondary fire, similar to primary for now
    AS_Character* Character = GetOwningSCharacter();
    AS_Weapon* Weapon = GetEquippedWeapon();
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();

    if (!Character || !Weapon || !WeaponData)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Similar aiming and effect logic as primary, but could use different parameters/effects
    FVector FireStartLocation;
    FVector FireDirection;
    AController* Controller = Character->GetController();
    if (Controller)
    {
        // Simplified aiming for example
        FVector MuzzleLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        FireStartLocation = MuzzleLocation;
        FireDirection = Controller->GetControlRotation().Vector();
    }
    else
    {
        FireStartLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        FireDirection = Weapon->GetActorForwardVector();
    }

    // The secondary fire might not use the same ExecuteFire or might pass different flags/data
    // For now, assume it *could* use ExecuteFire but potentially with different parameters
    // that would be fetched from WeaponData's secondary fire specific fields (if they existed)
    // or be intrinsic to the derived SecondaryFireAbility.
    // Weapon->ExecuteFire(FireStartLocation, FireDirection, nullptr, ... );

    UE_LOG(LogTemp, Warning, TEXT("US_WeaponSecondaryAbility::PerformWeaponSecondaryFire base called. Override in specific weapon secondary abilities."));
    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void US_WeaponSecondaryAbility::ApplyAmmoCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();
    if (WeaponData && WeaponData->AmmoCostEffect_Secondary && ActorInfo->AbilitySystemComponent.Get())
    {
        FGameplayEffectContextHandle ContextHandle = ActorInfo->AbilitySystemComponent->MakeEffectContext();
        ContextHandle.AddSourceObject(this);
        FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(WeaponData->AmmoCostEffect_Secondary, GetAbilityLevel(), ContextHandle);
        if (SpecHandle.IsValid())
        {
            ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
        }
    }
}

void US_WeaponSecondaryAbility::ApplyAbilityCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    // Similar to primary, ensure your Cooldown GE for secondary fire uses WeaponData->CooldownTag_Secondary
    Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
}