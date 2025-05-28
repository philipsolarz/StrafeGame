// Source/StrafeGame/Private/Abilities/Weapons/S_WeaponSecondaryAbility.cpp
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
    FGameplayTagContainer TempTags;
    TempTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Action.SecondaryFire")));
    SetAssetTags(TempTags);
    UE_LOG(LogTemp, Log, TEXT("US_WeaponSecondaryAbility::US_WeaponSecondaryAbility: Constructor for %s. InputID: %d"), *GetNameSafe(this), AbilityInputID);
}

bool US_WeaponSecondaryAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    bool bSuperCanActivate = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
    if (!bSuperCanActivate)
    {
        UE_LOG(LogTemp, Verbose, TEXT("US_WeaponSecondaryAbility::CanActivateAbility: %s - Super::CanActivateAbility returned false."), *GetNameSafe(this));
        return false;
    }
    const AS_Character* Character = GetOwningSCharacter();
    const AS_Weapon* EquippedWeapon = GetEquippedWeapon();
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();
    UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;

    if (!Character || !EquippedWeapon || !WeaponData || !ASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_WeaponSecondaryAbility::CanActivateAbility: %s - Missing Character (%d), Weapon (%d), WeaponData (%d), or ASC (%d)."), 
             *GetNameSafe(this), Character != nullptr, EquippedWeapon != nullptr, WeaponData != nullptr, ASC != nullptr);
        return false;
    }
    if (EquippedWeapon->GetCurrentWeaponState() != EWeaponState::Equipped)
    {
        UE_LOG(LogTemp, Verbose, TEXT("US_WeaponSecondaryAbility::CanActivateAbility: %s - Weapon %s is not in Equipped state (Current: %s)."), 
             *GetNameSafe(this), *EquippedWeapon->GetName(), *UEnum::GetValueAsString(EquippedWeapon->GetCurrentWeaponState()));
        return false;
    }
    if (!WeaponData->SecondaryFireAbilityClass) // This check seems odd here, as this IS the secondary fire ability. Maybe check if DA has it defined to allow this one.
    {
        UE_LOG(LogTemp, Warning, TEXT("US_WeaponSecondaryAbility::CanActivateAbility: %s - WeaponData %s has no SecondaryFireAbilityClass defined. This might be intended if ability is granted differently."), *GetNameSafe(this), *WeaponData->GetName());
        // This ability itself is the secondary fire. If the DA *must* list it, this is okay.
        // If the ability is granted directly, this check might not be needed.
        // For now, let's assume it means "is a secondary fire allowed at all by the data".
        // return false; 
    }
    if (WeaponData->AmmoAttribute.IsValid() && WeaponData->AmmoCostEffect_Secondary)
    {
        if (ASC->GetNumericAttribute(WeaponData->AmmoAttribute) <= 0) // Simplified check, could be more precise based on cost
        {
            UE_LOG(LogTemp, Log, TEXT("US_WeaponSecondaryAbility::CanActivateAbility: %s - Out of ammo for %s (Attribute: %s)."), 
                *GetNameSafe(this), *EquippedWeapon->GetName(), *WeaponData->AmmoAttribute.AttributeName);
            if (WeaponData->OutOfAmmoCueTag.IsValid())
            {
                ASC->ExecuteGameplayCue(WeaponData->OutOfAmmoCueTag, ASC->MakeEffectContext());
            }
            return false;
        }
    }
    UE_LOG(LogTemp, Verbose, TEXT("US_WeaponSecondaryAbility::CanActivateAbility: %s - Ability can be activated."), *GetNameSafe(this));
    return true;
}

void US_WeaponSecondaryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponSecondaryAbility::ActivateAbility: %s - Handle: %s, Actor: %s"), *GetNameSafe(this), *Handle.ToString(), ActorInfo ? *GetNameSafe(ActorInfo->AvatarActor.Get()) : TEXT("UnknownActor"));
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Log, TEXT("US_WeaponSecondaryAbility::ActivateAbility: %s - Failed to commit ability. Ending."), *GetNameSafe(this));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("US_WeaponSecondaryAbility::ActivateAbility: %s - Ability committed. Performing weapon secondary fire."), *GetNameSafe(this));
    PerformWeaponSecondaryFire(Handle, ActorInfo, ActivationInfo);
}

void US_WeaponSecondaryAbility::PerformWeaponSecondaryFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponSecondaryAbility::PerformWeaponSecondaryFire: %s - Base implementation called. Override in specific weapon abilities."), *GetNameSafe(this));
    // AS_Character* Character = GetOwningSCharacter();
    // AS_Weapon* Weapon = GetEquippedWeapon();
    // const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();

    // if (!Character || !Weapon || !WeaponData)
    // {
    //     UE_LOG(LogTemp, Error, TEXT("US_WeaponSecondaryAbility::PerformWeaponSecondaryFire: %s - Critical component missing. Ending."), *GetNameSafe(this));
    //     EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    //     return;
    // }

    // ApplyAmmoCost(Handle, ActorInfo, ActivationInfo); // Usually done by derived that know the cost
    // ApplyAbilityCooldown(Handle, ActorInfo, ActivationInfo); // Usually done by derived

    // FVector FireStartLocation;
    // FRotator CamRot; 
    // FVector FireDirection; 
    // AController* Controller = Character->GetController();

    // if (Controller)
    // {
    //     FVector MuzzleLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); 
    //     FVector CamLoc;
    //     Controller->GetPlayerViewPoint(CamLoc, CamRot); 
    //     FireDirection = CamRot.Vector(); 

    //     const float MaxTraceDist = WeaponData->MaxAimTraceRange > 0.f ? WeaponData->MaxAimTraceRange : 100000.0f; 
    //     FVector CamTraceEnd = CamLoc + FireDirection * MaxTraceDist;
    //     FHitResult CameraTraceHit;
    //     FCollisionQueryParams QueryParams;
    //     QueryParams.AddIgnoredActor(Character);
    //     QueryParams.AddIgnoredActor(Weapon);

    //     FireStartLocation = MuzzleLocation;
    //     if (GetWorld()->LineTraceSingleByChannel(CameraTraceHit, CamLoc, CamTraceEnd, ECC_Visibility, QueryParams))
    //     {
    //         FireDirection = (CameraTraceHit.ImpactPoint - MuzzleLocation).GetSafeNormal();
    //     }
    //     else {
    //         FireDirection = (CamTraceEnd - MuzzleLocation).GetSafeNormal();
    //     }
    // }
    // else
    // {
    //     FireStartLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); 
    //     FireDirection = Weapon->GetActorForwardVector();
    // }
    UE_LOG(LogTemp, Warning, TEXT("US_WeaponSecondaryAbility::PerformWeaponSecondaryFire base called for %s. This should be overridden by derived abilities to call Weapon->ExecuteFire or specific weapon functions."), *GetNameSafe(this));
    EndAbility(Handle, ActorInfo, ActivationInfo, false, false); // Base class doesn't fire, so end.
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
            UE_LOG(LogTemp, Log, TEXT("US_WeaponSecondaryAbility::ApplyAmmoCost: %s - Applied ammo cost effect %s."), *GetNameSafe(this), *WeaponData->AmmoCostEffect_Secondary->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("US_WeaponSecondaryAbility::ApplyAmmoCost: %s - Failed to create spec for ammo cost %s."), *GetNameSafe(this), *WeaponData->AmmoCostEffect_Secondary->GetName());
        }
    }
}

void US_WeaponSecondaryAbility::ApplyAbilityCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
    UE_LOG(LogTemp, Log, TEXT("US_WeaponSecondaryAbility::ApplyAbilityCooldown: %s - Cooldown applied."), *GetNameSafe(this));
}