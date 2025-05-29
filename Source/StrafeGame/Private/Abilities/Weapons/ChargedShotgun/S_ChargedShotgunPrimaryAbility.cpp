// Source/StrafeGame/Private/Abilities/Weapons/ChargedShotgun/S_ChargedShotgunPrimaryAbility.cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/Weapons/ChargedShotgun/S_ChargedShotgunPrimaryAbility.h"
#include "Weapons/ChargedShotgun/S_ChargedShotgun.h"
#include "Weapons/ChargedShotgun/S_ChargedShotgunDataAsset.h"
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Controller.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbility.h"

US_ChargedShotgunPrimaryAbility::US_ChargedShotgunPrimaryAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    ChargeInProgressTag = FGameplayTag::RequestGameplayTag(FName("State.Weapon.ChargedShotgun.PrimaryAbility.Charging"));
    // EarlyReleaseCooldownTag will be defined by the GE itself

    bIsCharging = false;
    bFiredDuringChargeLoop = false;
}

AS_ChargedShotgun* US_ChargedShotgunPrimaryAbility::GetChargedShotgun() const
{
    return Cast<AS_ChargedShotgun>(GetEquippedWeapon());
}

const US_ChargedShotgunDataAsset* US_ChargedShotgunPrimaryAbility::GetChargedShotgunData() const
{
    return Cast<US_ChargedShotgunDataAsset>(GetEquippedWeaponData());
}

bool US_ChargedShotgunPrimaryAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();
    if (ShotgunData && ShotgunData->SecondaryFireLockoutTag.IsValid())
    {
        if (ActorInfo->AbilitySystemComponent.IsValid() && ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(ShotgunData->SecondaryFireLockoutTag))
        {
            UE_LOG(LogTemp, Verbose, TEXT("US_ChargedShotgunPrimaryAbility::CanActivateAbility: Secondary fire lockout tag %s is active."), *ShotgunData->SecondaryFireLockoutTag.ToString());
            return false;
        }
    }
    return true;
}

void US_ChargedShotgunPrimaryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    // PerformWeaponFire is now called directly from here, not from Super's ActivateAbility
    // This is because Super::ActivateAbility will call PerformWeaponFire of US_WeaponPrimaryAbility
    // which is now generic. The specific logic for this ability starts in its own PerformWeaponFire.
}


void US_ChargedShotgunPrimaryAbility::PerformWeaponFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    // This method is called by US_WeaponPrimaryAbility::ActivateAbility after Commit.
    // For charged shotgun, this means starting the charge process.
    UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::PerformWeaponFire for %s - Starting charge logic."), *GetNameSafe(this));

    ResetAbilityState(); // Clear flags

    WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
    if (WaitInputReleaseTask)
    {
        // WaitInputReleaseTask->OnRelease.AddDynamic(this, &US_ChargedShotgunPrimaryAbility::InputReleased); // Already bound by Super or handled in this class's InputReleased
        WaitInputReleaseTask->ReadyForActivation();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("US_ChargedShotgunPrimaryAbility::PerformWeaponFire: Failed to create WaitInputReleaseTask. Cancelling ability."));
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    StartCharge();
}


void US_ChargedShotgunPrimaryAbility::StartCharge()
{
    if (bIsCharging)
    {
        UE_LOG(LogTemp, Verbose, TEXT("US_ChargedShotgunPrimaryAbility::StartCharge: Already charging."));
        return;
    }

    bIsCharging = true;
    bFiredDuringChargeLoop = false;

    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();

    if (!Shotgun || !ShotgunData || ShotgunData->PrimaryChargeTime <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_ChargedShotgunPrimaryAbility::StartCharge: Invalid Shotgun, Data, or ChargeTime. Cancelling."));
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
        return;
    }

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ChargeInProgressTag.IsValid() && ASC)
    {
        ASC->AddLooseGameplayTag(ChargeInProgressTag);
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::StartCharge: Added ChargeInProgressTag %s"), *ChargeInProgressTag.ToString());
    }
    if (ShotgunData->PrimaryChargeStartCue.IsValid() && ASC)
    {
        ASC->ExecuteGameplayCue(ShotgunData->PrimaryChargeStartCue, ASC->MakeEffectContext());
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::StartCharge: Executed PrimaryChargeStartCue %s"), *ShotgunData->PrimaryChargeStartCue.ToString());
    }
    Shotgun->K2_OnPrimaryChargeStart();

    ChargeTimerTask = UAbilityTask_WaitDelay::WaitDelay(this, ShotgunData->PrimaryChargeTime);
    if (ChargeTimerTask)
    {
        ChargeTimerTask->OnFinish.AddDynamic(this, &US_ChargedShotgunPrimaryAbility::OnChargeComplete);
        ChargeTimerTask->ReadyForActivation();
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::StartCharge: Charge timer started for %f seconds."), ShotgunData->PrimaryChargeTime);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("US_ChargedShotgunPrimaryAbility::StartCharge: Failed to create ChargeTimerTask. Cancelling."));
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
    }
}

void US_ChargedShotgunPrimaryAbility::OnChargeComplete()
{
    UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::OnChargeComplete. bIsCharging: %d"), bIsCharging);
    if (!bIsCharging)
    {
        return;
    }

    bIsCharging = false;
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ChargeInProgressTag.IsValid() && ASC)
    {
        ASC->RemoveLooseGameplayTag(ChargeInProgressTag);
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::OnChargeComplete: Removed ChargeInProgressTag %s"), *ChargeInProgressTag.ToString());
    }

    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    if (Shotgun) Shotgun->K2_OnPrimaryChargeComplete();

    if (CommitAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo()))
    {
        DoFire(); // Fire the weapon
        bFiredDuringChargeLoop = true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("US_ChargedShotgunPrimaryAbility::OnChargeComplete: Failed to commit ability post-charge. Ending."));
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
        return;
    }

    bool bIsInputStillPressed = false;
    const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
    if (ActorInfo && ActorInfo->IsLocallyControlled())
    {
        if (const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
        {
            bIsInputStillPressed = Spec->InputPressed;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::OnChargeComplete: Input still pressed: %d"), bIsInputStillPressed);

    if (WaitInputReleaseTask && WaitInputReleaseTask->IsActive() && bIsInputStillPressed)
    {
        if (CanActivateAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), nullptr, nullptr, nullptr))
        {
            UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::OnChargeComplete: Input held, restarting charge."));
            StartCharge();
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::OnChargeComplete: Input held, but CanActivateAbility is false for restart. Ending."));
            EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::OnChargeComplete: Input released or task inactive. Ending."));
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
    }
}

void US_ChargedShotgunPrimaryAbility::DoFire()
{
    AS_Character* Character = GetOwningSCharacter();
    AS_ChargedShotgun* Weapon = GetChargedShotgun();
    const US_ChargedShotgunDataAsset* WeaponData = GetChargedShotgunData();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    if (!Character || !Weapon || !WeaponData || !ASC)
    {
        UE_LOG(LogTemp, Error, TEXT("US_ChargedShotgunPrimaryAbility::DoFire: Precondition failed. Character: %d, Weapon: %d, WeaponData: %d, ASC: %d"),
            Character != nullptr, Weapon != nullptr, WeaponData != nullptr, ASC != nullptr);
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::DoFire for %s"), *Weapon->GetName());

    Super::ApplyAmmoCost(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo());
    Super::ApplyAbilityCooldown(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo());

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

        const float MaxFireDistance = WeaponData->PrimaryFireHitscanRange;
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
    else
    {
        FireStartLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        FireDirection = Weapon->GetActorForwardVector();
    }

    // The weapon itself will fetch PelletCount, Spread, Range, Damage from its DataAsset
    Weapon->ExecutePrimaryFire(FireStartLocation, FireDirection, CurrentEventData ? *CurrentEventData : FGameplayEventData());

    if (WeaponData->MuzzleFlashCueTag.IsValid())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        CueParams.Normal = FireDirection;
        CueParams.Instigator = Character;
        CueParams.EffectCauser = Weapon;
        ASC->ExecuteGameplayCue(WeaponData->MuzzleFlashCueTag, CueParams);
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::DoFire: Executed MuzzleFlashCue %s"), *WeaponData->MuzzleFlashCueTag.ToString());
    }

    if (WeaponData->PrimaryChargeCompleteCue.IsValid())
    {
        ASC->ExecuteGameplayCue(WeaponData->PrimaryChargeCompleteCue, ASC->MakeEffectContext());
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::DoFire: Executed PrimaryChargeCompleteCue %s"), *WeaponData->PrimaryChargeCompleteCue.ToString());
    }
    UE_LOG(LogTemp, Log, TEXT("Charged Shotgun Primary Fired (Ability: %s)"), *GetNameSafe(this));
}

void US_ChargedShotgunPrimaryAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo);
    UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::InputReleased. bIsCharging: %d"), bIsCharging);

    if (bIsCharging) // If input released DURING the charge timer
    {
        ApplyEarlyReleasePenalty();
        AS_ChargedShotgun* Shotgun = GetChargedShotgun();
        if (Shotgun) Shotgun->K2_OnPrimaryChargeCancelled();
    }

    if (IsActive())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, bIsCharging); // bWasCancelled is true if released during charge
    }
}

void US_ChargedShotgunPrimaryAbility::ApplyEarlyReleasePenalty()
{
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    if (ShotgunData && ShotgunData->PrimaryEarlyReleaseCooldownEffect && ASC)
    {
        FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
        ContextHandle.AddSourceObject(this);
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ShotgunData->PrimaryEarlyReleaseCooldownEffect, GetAbilityLevel(), ContextHandle);
        if (SpecHandle.IsValid())
        {
            ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), SpecHandle);
            UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility: Applied Early Release Penalty Cooldown Effect %s."), *ShotgunData->PrimaryEarlyReleaseCooldownEffect->GetName());
        }
    }
}

void US_ChargedShotgunPrimaryAbility::ResetAbilityState()
{
    bIsCharging = false;
    bFiredDuringChargeLoop = false;

    if (ChargeTimerTask && ChargeTimerTask->IsActive())
    {
        ChargeTimerTask->EndTask();
    }
    ChargeTimerTask = nullptr;

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ChargeInProgressTag.IsValid() && ASC && ASC->HasMatchingGameplayTag(ChargeInProgressTag))
    {
        ASC->RemoveLooseGameplayTag(ChargeInProgressTag);
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::ResetAbilityState: Removed ChargeInProgressTag %s"), *ChargeInProgressTag.ToString());
    }
}

void US_ChargedShotgunPrimaryAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunPrimaryAbility::EndAbility for %s. bWasCancelled: %d"), *GetNameSafe(this), bWasCancelled);
    ResetAbilityState();

    if (WaitInputReleaseTask && WaitInputReleaseTask->IsActive())
    {
        WaitInputReleaseTask->EndTask();
    }
    WaitInputReleaseTask = nullptr;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}