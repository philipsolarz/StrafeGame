// Source/StrafeGame/Private/Abilities/Weapons/S_WeaponPrimaryAbility.cpp
#include "Abilities/Weapons/S_WeaponPrimaryAbility.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Player/S_Character.h"
#include "Player/Attributes/S_AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Controller.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbility.h"

US_WeaponPrimaryAbility::US_WeaponPrimaryAbility()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    AbilityInputID = 0; // Typically 0 for primary fire
    FGameplayTagContainer TempTags;
    TempTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Action.PrimaryFire")));
    SetAssetTags(TempTags);
    UE_LOG(LogTemp, Log, TEXT("US_WeaponPrimaryAbility::US_WeaponPrimaryAbility: Constructor for %s. InputID: %d"), *GetNameSafe(this), AbilityInputID);
}

bool US_WeaponPrimaryAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    bool bSuperCanActivate = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
    if (!bSuperCanActivate)
    {
        UE_LOG(LogTemp, Verbose, TEXT("US_WeaponPrimaryAbility::CanActivateAbility: %s - Super::CanActivateAbility returned false."), *GetNameSafe(this));
        return false;
    }

    const AS_Character* Character = GetOwningSCharacter();
    const AS_Weapon* EquippedWeapon = GetEquippedWeapon();
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();
    UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;

    if (!Character || !EquippedWeapon || !WeaponData || !ASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_WeaponPrimaryAbility::CanActivateAbility: %s - Missing Character (%d), Weapon (%d), WeaponData (%d), or ASC (%d)."),
            *GetNameSafe(this), Character != nullptr, EquippedWeapon != nullptr, WeaponData != nullptr, ASC != nullptr);
        return false;
    }

    if (EquippedWeapon->GetCurrentWeaponState() != EWeaponState::Equipped)
    {
        UE_LOG(LogTemp, Verbose, TEXT("US_WeaponPrimaryAbility::CanActivateAbility: %s - Weapon %s is not in Equipped state (Current: %s)."),
            *GetNameSafe(this), *EquippedWeapon->GetName(), *UEnum::GetValueAsString(EquippedWeapon->GetCurrentWeaponState()));
        return false;
    }

    if (WeaponData->AmmoAttribute.IsValid())
    {
        if (WeaponData->AmmoCostEffect_Primary)
        {
            if (ASC->GetNumericAttribute(WeaponData->AmmoAttribute) <= 0)
            {
                UE_LOG(LogTemp, Log, TEXT("US_WeaponPrimaryAbility::CanActivateAbility: %s - Out of ammo for %s (Attribute: %s)."),
                    *GetNameSafe(this), *EquippedWeapon->GetName(), *WeaponData->AmmoAttribute.AttributeName);
                if (WeaponData->OutOfAmmoCueTag.IsValid())
                {
                    ASC->ExecuteGameplayCue(WeaponData->OutOfAmmoCueTag, ASC->MakeEffectContext());
                }
                return false;
            }
        }
    }
    UE_LOG(LogTemp, Verbose, TEXT("US_WeaponPrimaryAbility::CanActivateAbility: %s - Ability can be activated."), *GetNameSafe(this));
    return true;
}

void US_WeaponPrimaryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponPrimaryAbility::ActivateAbility: %s (%s) - Handle: %s"), *GetClass()->GetName(), *GetNameSafe(this), *Handle.ToString());
    CurrentEventData = TriggerEventData;

    // Call UGameplayAbility's ActivateAbility. This is important for BP K2_ActivateAbility etc.
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // Check if the ability is still active. K2_ActivateAbility (if implemented in BP)
    // could have called EndAbility.
    if (IsActive())
    {
        if (CommitAbility(Handle, ActorInfo, ActivationInfo))
        {
            UE_LOG(LogTemp, Log, TEXT("US_WeaponPrimaryAbility::ActivateAbility: %s (%s) - Committed. Calling PerformWeaponFire."), *GetClass()->GetName(), *GetNameSafe(this));
            // PerformWeaponFire is virtual, so the most derived version will be called
            // (e.g., US_ChargedShotgunPrimaryAbility::PerformWeaponFire if this is an instance of that)
            PerformWeaponFire(Handle, ActorInfo, ActivationInfo);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("US_WeaponPrimaryAbility::ActivateAbility: %s (%s) - Commit failed. Ending ability."), *GetClass()->GetName(), *GetNameSafe(this));
            EndAbility(Handle, ActorInfo, ActivationInfo, true, true); // true for replicate, true for cancelled
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("US_WeaponPrimaryAbility::ActivateAbility: %s (%s) - Ability was ended during Super::ActivateAbility or K2_ActivateAbility. Not proceeding with PerformWeaponFire."), *GetClass()->GetName(), *GetNameSafe(this));
    }
}


void US_WeaponPrimaryAbility::PerformWeaponFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    AS_Character* Character = GetOwningSCharacter();
    AS_Weapon* Weapon = GetEquippedWeapon();
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();
    UE_LOG(LogTemp, Log, TEXT("US_WeaponPrimaryAbility::PerformWeaponFire (Base): %s - Character: %s, Weapon: %s, WeaponData: %s"),
        *GetNameSafe(this), *GetNameSafe(Character), *GetNameSafe(Weapon), *GetNameSafe(WeaponData));

    if (!Character || !Weapon || !WeaponData)
    {
        UE_LOG(LogTemp, Error, TEXT("US_WeaponPrimaryAbility::PerformWeaponFire (Base): %s - Critical component missing. Ending ability."), *GetNameSafe(this));
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    ApplyAmmoCost(Handle, ActorInfo, ActivationInfo);
    ApplyAbilityCooldown(Handle, ActorInfo, ActivationInfo);

    FVector FireStartLocation;
    FRotator CamRot;
    FVector FireDirection;

    AController* Controller = Character->GetController();
    if (Controller)
    {
        FVector MuzzleLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        FVector CamLoc;
        Controller->GetPlayerViewPoint(CamLoc, CamRot);
        FireDirection = CamRot.Vector();

        const float MaxTraceDist = WeaponData->MaxAimTraceRange > 0.f ? WeaponData->MaxAimTraceRange : 100000.0f;
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
        else
        {
            FireDirection = (CamTraceEnd - MuzzleLocation).GetSafeNormal();
        }
        UE_LOG(LogTemp, Verbose, TEXT("US_WeaponPrimaryAbility::PerformWeaponFire (Base): %s - Firing from Muzzle: %s, Dir: %s (Player Aim)"), *GetNameSafe(this), *MuzzleLocation.ToString(), *FireDirection.ToString());
    }
    else
    {
        FireStartLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        FireDirection = Weapon->GetActorForwardVector();
        UE_LOG(LogTemp, Verbose, TEXT("US_WeaponPrimaryAbility::PerformWeaponFire (Base): %s - Firing from Muzzle: %s, Dir: %s (Actor Forward - No Controller)"), *GetNameSafe(this), *FireStartLocation.ToString(), *FireDirection.ToString());
    }

    FireMontageTask = PlayWeaponMontage(WeaponData->FireMontage);
    if (FireMontageTask)
    {
        UE_LOG(LogTemp, Log, TEXT("US_WeaponPrimaryAbility::PerformWeaponFire (Base): %s - FireMontageTask started."), *GetNameSafe(this));
        FireMontageTask->OnCompleted.AddDynamic(this, &US_WeaponPrimaryAbility::OnFireMontageCompleted);
        FireMontageTask->OnInterrupted.AddDynamic(this, &US_WeaponPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        FireMontageTask->OnCancelled.AddDynamic(this, &US_WeaponPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        FireMontageTask->ReadyForActivation();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("US_WeaponPrimaryAbility::PerformWeaponFire (Base): %s - No FireMontageTask. Firing directly and ending."), *GetNameSafe(this));
        const FGameplayEventData* AbilityTriggerData = CurrentEventData;
        UE_LOG(LogTemp, Verbose, TEXT("US_WeaponPrimaryAbility::PerformWeaponFire (Base): %s - Calling Weapon->ExecutePrimaryFire directly."), *GetNameSafe(this));
        Weapon->ExecutePrimaryFire(
            FireStartLocation,
            FireDirection,
            AbilityTriggerData ? *AbilityTriggerData : FGameplayEventData()
        );
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (WeaponData->MuzzleFlashCueTag.IsValid() && ASC)
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        CueParams.Normal = FireDirection;
        CueParams.Instigator = Character;
        CueParams.EffectCauser = Weapon;
        ASC->ExecuteGameplayCue(WeaponData->MuzzleFlashCueTag, CueParams);
        UE_LOG(LogTemp, Verbose, TEXT("US_WeaponPrimaryAbility::PerformWeaponFire (Base): %s - Executed MuzzleFlashCue %s."), *GetNameSafe(this), *WeaponData->MuzzleFlashCueTag.ToString());
    }
    if (!FireMontageTask)
    {
        // Already ended if no montage task
    }
}

void US_WeaponPrimaryAbility::ApplyAmmoCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (WeaponData && WeaponData->AmmoCostEffect_Primary && ASC)
    {
        FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
        ContextHandle.AddSourceObject(this);
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(WeaponData->AmmoCostEffect_Primary, GetAbilityLevel(), ContextHandle);
        if (SpecHandle.IsValid())
        {
            ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
            UE_LOG(LogTemp, Log, TEXT("US_WeaponPrimaryAbility::ApplyAmmoCost: %s - Applied ammo cost effect %s."), *GetNameSafe(this), *WeaponData->AmmoCostEffect_Primary->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("US_WeaponPrimaryAbility::ApplyAmmoCost: %s - Failed to create spec for ammo cost %s."), *GetNameSafe(this), *WeaponData->AmmoCostEffect_Primary->GetName());
        }
    }
}

void US_WeaponPrimaryAbility::ApplyAbilityCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    const UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
    if (CooldownGE)
    {
        ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, CooldownGE, GetAbilityLevel());
        UE_LOG(LogTemp, Log, TEXT("US_WeaponPrimaryAbility::ApplyAbilityCooldown: %s - Applied Cooldown GE %s."), *GetNameSafe(this), *CooldownGE->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("US_WeaponPrimaryAbility::ApplyAbilityCooldown: %s - No CooldownGameplayEffect set for this ability."), *GetNameSafe(this));
    }
}

void US_WeaponPrimaryAbility::OnFireMontageCompleted()
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponPrimaryAbility::OnFireMontageCompleted: %s - Ending ability."), *GetNameSafe(this));
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void US_WeaponPrimaryAbility::OnFireMontageInterruptedOrCancelled()
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponPrimaryAbility::OnFireMontageInterruptedOrCancelled: %s - Ending ability (was cancelled/interrupted)."), *GetNameSafe(this));
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}