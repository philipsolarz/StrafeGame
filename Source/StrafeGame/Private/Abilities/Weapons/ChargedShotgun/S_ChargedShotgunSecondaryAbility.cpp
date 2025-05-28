#include "Abilities/Weapons/ChargedShotgun/S_ChargedShotgunSecondaryAbility.h"
#include "Weapons/ChargedShotgun/S_ChargedShotgun.h"
#include "Weapons/ChargedShotgun/S_ChargedShotgunDataAsset.h" // You'll need to create this
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Controller.h"

US_ChargedShotgunSecondaryAbility::US_ChargedShotgunSecondaryAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted; // Or ServerInitiated if charge state is complex server-side

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
    if (!ShotgunData || !ShotgunData->SecondaryFireLockoutTag.IsValid()) return true; // No lockout tag defined, allow activation

    if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(ShotgunData->SecondaryFireLockoutTag))
    {
        return false; // Cannot activate if locked out
    }
    return true;
}

void US_ChargedShotgunSecondaryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!HasEnoughResourcesToActivate())
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

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
    bInputReleasedDuringChargeAttempt = false; // Reset this at charge start

    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();

    if (!Shotgun || !ShotgunData || ShotgunData->SecondaryChargeTime <= 0.f)
    {
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
        return;
    }

    if (ChargeInProgressTag.IsValid()) GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(ChargeInProgressTag);
    if (ShotgunData->SecondaryChargeStartCue.IsValid()) GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(ShotgunData->SecondaryChargeStartCue, GetAbilitySystemComponentFromActorInfo()->MakeEffectContext());
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
    if (!bIsCharging) return; // Could have been cancelled

    bIsCharging = false;
    if (ChargeInProgressTag.IsValid()) GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(ChargeInProgressTag);

    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();

    if (bInputReleasedDuringChargeAttempt) // If input was released *while* the charge timer was ticking
    {
        // No penalty, just end. This path means player let go before charge finished.
        if (Shotgun) Shotgun->K2_OnSecondaryChargeCancelled();
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true); // true for cancelled
        return;
    }

    // Charge finished successfully
    bOverchargedShotStored = true;
    if (OverchargedStateTag.IsValid()) GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(OverchargedStateTag);
    if (ShotgunData && ShotgunData->SecondaryChargeHeldCue.IsValid()) GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(ShotgunData->SecondaryChargeHeldCue, GetAbilitySystemComponentFromActorInfo()->MakeEffectContext());
    if (Shotgun) Shotgun->K2_OnSecondaryChargeHeld();

    // Now we wait for input release via the WaitInputReleaseTask / InputReleased override.
    // If input is already released by now (very fast tap), InputReleased might handle it.
}

void US_ChargedShotgunSecondaryAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo);

    if (bIsCharging) // If input released *during* the charge phase
    {
        bInputReleasedDuringChargeAttempt = true; // Mark that input was released. OnSecondaryChargeComplete will check this.
        // No immediate penalty, just let the charge timer run out or handle in OnSecondaryChargeComplete.
        // If the ability should end immediately on early release (no charge stored):
        // {
        //     AS_ChargedShotgun* Shotgun = GetChargedShotgun();
        //     if(Shotgun) Shotgun->K2_OnSecondaryChargeCancelled();
        //     EndAbility(Handle, ActorInfo, ActivationInfo, false, true); // true for cancelled
        // }
        // For now, we let OnSecondaryChargeComplete decide based on this flag.
        return;
    }

    if (bOverchargedShotStored) // If input released *after* charge was stored
    {
        AttemptFireOverchargedShot();
        // EndAbility is called within AttemptFireOverchargedShot or after it
    }
    else
    {
        // Input released but not charging and no shot stored (e.g. released before charge started if activation was slow)
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
    }
}

void US_ChargedShotgunSecondaryAbility::AttemptFireOverchargedShot()
{
    if (!bOverchargedShotStored) return;

    AS_Character* Character = GetOwningSCharacter();
    AS_ChargedShotgun* Weapon = GetChargedShotgun();
    const US_ChargedShotgunDataAsset* WeaponData = GetChargedShotgunData();

    if (!Character || !Weapon || !WeaponData || !GetAbilityActorInfo())
    {
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
        return;
    }

    // 1. Apply Ammo Cost
    Super::ApplyAmmoCost(GetCurrentAbilitySpecHandle(), GetAbilityActorInfo(), GetCurrentActivationInfo()); // Uses WeaponData->AmmoCostEffect_Secondary

    // 2. Apply Lockout Cooldown
    ApplyWeaponLockoutCooldown();

    // 3. Fire Logic (similar to primary, but using secondary parameters)
    FVector FireStartLocation;
    FVector FireDirection;
    AController* Controller = Character->GetController();
    if (Controller)
    {
        FVector MuzzleLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        FHitResult CameraTraceHit;
        FVector CamLoc, CamDir;
        Controller->GetPlayerViewPoint(CamLoc, CamDir);
        CamDir = Controller->GetControlRotation().Vector();
        const float MaxFireDistance = WeaponData->SecondaryFireHitscanRange; // Use specific range
        FVector CamTraceEnd = CamLoc + CamDir * MaxFireDistance;
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
        FireStartLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        FireDirection = Weapon->GetActorForwardVector();
    }

    Weapon->ExecuteFire(FireStartLocation, FireDirection, nullptr, WeaponData->SecondaryFireSpreadAngle, WeaponData->SecondaryFireHitscanRange, nullptr);

    if (WeaponData->MuzzleFlashCueTag.IsValid()) GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(WeaponData->MuzzleFlashCueTag, GetAbilitySystemComponentFromActorInfo()->MakeEffectContext());
    Weapon->K2_OnSecondaryChargeReleased();

    bOverchargedShotStored = false;
    if (OverchargedStateTag.IsValid()) GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(OverchargedStateTag);

    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}

void US_ChargedShotgunSecondaryAbility::ApplyWeaponLockoutCooldown()
{
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    if (ShotgunData && ShotgunData->SecondaryFireLockoutEffect && ASC) // Assuming SecondaryFireLockoutEffect is on DataAsset
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