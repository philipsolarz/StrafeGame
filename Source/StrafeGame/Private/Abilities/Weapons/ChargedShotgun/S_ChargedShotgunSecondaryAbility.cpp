#include "Abilities/Weapons/ChargedShotgun/S_ChargedShotgunSecondaryAbility.h"
#include "Weapons/ChargedShotgun/S_ChargedShotgun.h"
#include "Weapons/ChargedShotgun/S_ChargedShotgunDataAsset.h" 
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Controller.h"
#include "GameplayEffectTypes.h" 

US_ChargedShotgunSecondaryAbility::US_ChargedShotgunSecondaryAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    ChargeInProgressTag = FGameplayTag::RequestGameplayTag(FName("State.Weapon.Charging.Secondary.Shotgun"));
    OverchargedStateTag = FGameplayTag::RequestGameplayTag(FName("State.Weapon.Overcharged.Secondary.Shotgun"));

    bIsCharging = false;
    bOverchargedShotStored = false;
    bInputReleasedDuringChargeAttempt = false;
}

AS_ChargedShotgun* US_ChargedShotgunSecondaryAbility::GetChargedShotgun() const
{
    return Cast<AS_ChargedShotgun>(GetEquippedWeapon());
}

const US_ChargedShotgunDataAsset* US_ChargedShotgunSecondaryAbility::GetChargedShotgunData() const
{
    return Cast<US_ChargedShotgunDataAsset>(GetEquippedWeaponData());
}

bool US_ChargedShotgunSecondaryAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();
    if (!ShotgunData || !ShotgunData->SecondaryFireLockoutTag.IsValid()) return true;

    if (ActorInfo->AbilitySystemComponent.IsValid() && ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(ShotgunData->SecondaryFireLockoutTag))
    {
        return false;
    }
    return true;
}

void US_ChargedShotgunSecondaryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    ResetAbilityState();

    WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
    if (WaitInputReleaseTask)
    {
        WaitInputReleaseTask->ReadyForActivation();
    }
    else
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    StartSecondaryCharge();
}

void US_ChargedShotgunSecondaryAbility::StartSecondaryCharge()
{
    if (bIsCharging || bOverchargedShotStored) return;

    bIsCharging = true;
    bInputReleasedDuringChargeAttempt = false;

    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();

    if (!Shotgun || !ShotgunData || ShotgunData->SecondaryChargeTime <= 0.f)
    {
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
        return;
    }

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ChargeInProgressTag.IsValid() && ASC) ASC->AddLooseGameplayTag(ChargeInProgressTag);
    if (ShotgunData->SecondaryChargeStartCue.IsValid() && ASC) ASC->ExecuteGameplayCue(ShotgunData->SecondaryChargeStartCue, ASC->MakeEffectContext());
    Shotgun->K2_OnSecondaryChargeStart();

    ChargeTimerTask = UAbilityTask_WaitDelay::WaitDelay(this, ShotgunData->SecondaryChargeTime);
    if (ChargeTimerTask)
    {
        ChargeTimerTask->OnFinish.AddDynamic(this, &US_ChargedShotgunSecondaryAbility::OnSecondaryChargeComplete);
        ChargeTimerTask->ReadyForActivation();
    }
    else
    {
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
    }
}

void US_ChargedShotgunSecondaryAbility::OnSecondaryChargeComplete()
{
    if (!bIsCharging) return;

    bIsCharging = false;
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ChargeInProgressTag.IsValid() && ASC) ASC->RemoveLooseGameplayTag(ChargeInProgressTag);

    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();

    if (bInputReleasedDuringChargeAttempt)
    {
        if (Shotgun) Shotgun->K2_OnSecondaryChargeCancelled();
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
        return;
    }

    bOverchargedShotStored = true;
    if (OverchargedStateTag.IsValid() && ASC) ASC->AddLooseGameplayTag(OverchargedStateTag);
    if (ShotgunData && ShotgunData->SecondaryChargeHeldCue.IsValid() && ASC) ASC->ExecuteGameplayCue(ShotgunData->SecondaryChargeHeldCue, ASC->MakeEffectContext());
    if (Shotgun) Shotgun->K2_OnSecondaryChargeHeld();
}

void US_ChargedShotgunSecondaryAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo);

    if (bIsCharging)
    {
        bInputReleasedDuringChargeAttempt = true;
        return;
    }

    if (bOverchargedShotStored)
    {
        if (CommitAbility(Handle, ActorInfo, ActivationInfo))
        {
            AttemptFireOverchargedShot();
        }
        else
        {
            AS_ChargedShotgun* Shotgun = GetChargedShotgun();
            if (Shotgun) Shotgun->K2_OnSecondaryChargeCancelled();
            CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        }
    }
    else
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
    }
}

void US_ChargedShotgunSecondaryAbility::AttemptFireOverchargedShot()
{
    if (!bOverchargedShotStored) return;

    AS_Character* Character = GetOwningSCharacter();
    AS_ChargedShotgun* Weapon = GetChargedShotgun();
    const US_ChargedShotgunDataAsset* WeaponData = GetChargedShotgunData();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    if (!Character || !Weapon || !WeaponData || !ASC)
    {
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
        return;
    }

    Super::ApplyAmmoCost(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo());
    ApplyWeaponLockoutCooldown();

    FVector FireStartLocation;
    FRotator CamRot; // CORRECTED
    FVector FireDirection;
    AController* Controller = Character->GetController();
    if (Controller)
    {
        FVector MuzzleLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); // CORRECTED
        FVector CamLoc;
        Controller->GetPlayerViewPoint(CamLoc, CamRot); // CORRECTED
        FireDirection = CamRot.Vector();

        const float MaxFireDistance = WeaponData->SecondaryFireHitscanRange;
        FVector CamTraceEnd = CamLoc + FireDirection * MaxFireDistance;
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
    else {
        FireStartLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); // CORRECTED
        FireDirection = Weapon->GetActorForwardVector();
    }

    const FGameplayEventData* AbilityTriggerData = GetCurrentAbilityTriggerData(); // CORRECTED
    Weapon->ExecuteFire(FireStartLocation, FireDirection, AbilityTriggerData ? *AbilityTriggerData : FGameplayEventData(), WeaponData->SecondaryFireSpreadAngle, WeaponData->SecondaryFireHitscanRange, nullptr);

    if (WeaponData->SecondaryOverchargedFireCue.IsValid()) ASC->ExecuteGameplayCue(WeaponData->SecondaryOverchargedFireCue, ASC->MakeEffectContext());
    else if (WeaponData->MuzzleFlashCueTag.IsValid()) ASC->ExecuteGameplayCue(WeaponData->MuzzleFlashCueTag, ASC->MakeEffectContext());

    Weapon->K2_OnSecondaryChargeReleased();

    bOverchargedShotStored = false;
    if (OverchargedStateTag.IsValid()) ASC->RemoveLooseGameplayTag(OverchargedStateTag);

    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}

void US_ChargedShotgunSecondaryAbility::ApplyWeaponLockoutCooldown()
{
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    if (ShotgunData && ShotgunData->SecondaryFireLockoutEffect && ASC)
    {
        FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
        ContextHandle.AddSourceObject(this);
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ShotgunData->SecondaryFireLockoutEffect, GetAbilityLevel(), ContextHandle);
        if (SpecHandle.IsValid())
        {
            ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), SpecHandle);
            UE_LOG(LogTemp, Log, TEXT("Charged Shotgun Secondary: Applied Weapon Lockout Cooldown."));
        }
    }
}

void US_ChargedShotgunSecondaryAbility::ResetAbilityState()
{
    bIsCharging = false;
    bOverchargedShotStored = false;
    bInputReleasedDuringChargeAttempt = false;

    if (ChargeTimerTask && ChargeTimerTask->IsActive()) ChargeTimerTask->EndTask();
    ChargeTimerTask = nullptr;

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC)
    {
        if (ChargeInProgressTag.IsValid()) ASC->RemoveLooseGameplayTag(ChargeInProgressTag);
        if (OverchargedStateTag.IsValid()) ASC->RemoveLooseGameplayTag(OverchargedStateTag);
    }
}

void US_ChargedShotgunSecondaryAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    ResetAbilityState();
    if (WaitInputReleaseTask && WaitInputReleaseTask->IsActive())
    {
        WaitInputReleaseTask->EndTask();
    }
    WaitInputReleaseTask = nullptr;
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}