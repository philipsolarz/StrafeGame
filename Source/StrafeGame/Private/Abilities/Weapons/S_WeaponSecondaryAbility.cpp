#include "Abilities/Weapons/S_WeaponSecondaryAbility.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Player/S_Character.h"
#include "Player/Attributes/S_AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Controller.h"
#include "GameplayEffectTypes.h" // For FGameplayEventData

US_WeaponSecondaryAbility::US_WeaponSecondaryAbility()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    AbilityInputID = 1;
    AssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Action.SecondaryFire"))); // CORRECTED
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
    if (!WeaponData->SecondaryFireAbilityClass)
    {
        return false;
    }
    if (WeaponData->AmmoAttribute.IsValid() && WeaponData->AmmoCostEffect_Secondary)
    {
        // A more precise check would be:
        // if (ASC->GetNumericAttribute(WeaponData->AmmoAttribute) < GetAmmoCost(WeaponData->AmmoCostEffect_Secondary))
        // For now, just checking if > 0
        if (ASC->GetNumericAttribute(WeaponData->AmmoAttribute) <= 0)
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
    AS_Character* Character = GetOwningSCharacter();
    AS_Weapon* Weapon = GetEquippedWeapon();
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();

    if (!Character || !Weapon || !WeaponData)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FVector FireStartLocation;
    FRotator CamRot; // CORRECTED
    FVector FireDirection; // Renamed from CamAimDir for clarity
    AController* Controller = Character->GetController();

    if (Controller)
    {
        FVector MuzzleLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); // CORRECTED
        FVector CamLoc;
        Controller->GetPlayerViewPoint(CamLoc, CamRot); // CORRECTED
        FireDirection = CamRot.Vector(); // Initial direction

        const float MaxTraceDist = WeaponData->MaxAimTraceRange > 0.f ? WeaponData->MaxAimTraceRange : 100000.0f; // CORRECTED
        FVector CamTraceEnd = CamLoc + FireDirection * MaxTraceDist;
        FHitResult CameraTraceHit;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Character);
        QueryParams.AddIgnoredActor(Weapon);

        FireStartLocation = MuzzleLocation;
        if (GetWorld()->LineTraceSingleByChannel(CameraTraceHit, CamLoc, CamTraceEnd, ECC_Visibility, QueryParams))
        {
            FireDirection = (CameraTraceHit.ImpactPoint - MuzzleLocation).GetSafeNormal();
        }
        else {
            FireDirection = (CamTraceEnd - MuzzleLocation).GetSafeNormal();
        }
    }
    else
    {
        FireStartLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); // CORRECTED
        FireDirection = Weapon->GetActorForwardVector();
    }

    // Base class doesn't call ExecuteFire. Derived classes like RocketLauncherSecondary will.
    UE_LOG(LogTemp, Warning, TEXT("US_WeaponSecondaryAbility::PerformWeaponSecondaryFire base called. Override in specific weapon secondary abilities."));
    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void US_WeaponSecondaryAbility::ApplyAmmoCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (WeaponData && WeaponData->AmmoCostEffect_Secondary && ASC)
    {
        FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
        ContextHandle.AddSourceObject(this);
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(WeaponData->AmmoCostEffect_Secondary, GetAbilityLevel(), ContextHandle);
        if (SpecHandle.IsValid())
        {
            ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
        }
    }
}

void US_WeaponSecondaryAbility::ApplyAbilityCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
}