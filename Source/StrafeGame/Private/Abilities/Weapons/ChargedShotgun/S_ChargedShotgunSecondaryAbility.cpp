// Source/StrafeGame/Private/Abilities/Weapons/ChargedShotgun/S_ChargedShotgunSecondaryAbility.cpp
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
#include "Abilities/GameplayAbility.h"
#include "TimerManager.h"
#include "Engine/World.h"

US_ChargedShotgunSecondaryAbility::US_ChargedShotgunSecondaryAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    ChargeInProgressTag = FGameplayTag::RequestGameplayTag(FName("State.Weapon.ChargedShotgun.SecondaryAbility.Charging"));
    OverchargedStateTag = FGameplayTag::RequestGameplayTag(FName("State.Weapon.ChargedShotgun.SecondaryAbility.Overcharged"));

    bIsCharging = false;
    bOverchargedShotStored = false;
    bInputReleasedDuringChargeAttempt = false;
    ChargeStartTime = 0.0f;
    ChargeDuration = 0.0f;
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
    if (ShotgunData && ShotgunData->SecondaryFireLockoutTag.IsValid()) // Check against the same lockout tag
    {
        if (ActorInfo->AbilitySystemComponent.IsValid() && ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(ShotgunData->SecondaryFireLockoutTag))
        {
            UE_LOG(LogTemp, Verbose, TEXT("US_ChargedShotgunSecondaryAbility::CanActivateAbility: Secondary fire lockout tag %s is active."), *ShotgunData->SecondaryFireLockoutTag.ToString());
            return false;
        }
    }
    return true;
}

void US_ChargedShotgunSecondaryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    // PerformWeaponSecondaryFire is called by base ActivateAbility after commit.
}

void US_ChargedShotgunSecondaryAbility::PerformWeaponSecondaryFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::PerformWeaponSecondaryFire for %s - Starting charge logic."), *GetNameSafe(this));
    ResetAbilityState();

    WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
    if (WaitInputReleaseTask)
    {
        // WaitInputReleaseTask->OnRelease.AddDynamic(this, &US_ChargedShotgunSecondaryAbility::InputReleased); // Base class might handle this or we override
        WaitInputReleaseTask->ReadyForActivation();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("US_ChargedShotgunSecondaryAbility::PerformWeaponSecondaryFire: Failed to create WaitInputReleaseTask. Cancelling ability."));
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    StartSecondaryCharge();
}

void US_ChargedShotgunSecondaryAbility::StartSecondaryCharge()
{
    if (bIsCharging || bOverchargedShotStored)
    {
        UE_LOG(LogTemp, Verbose, TEXT("US_ChargedShotgunSecondaryAbility::StartSecondaryCharge: Already charging or overcharged shot stored."));
        return;
    }

    bIsCharging = true;
    bInputReleasedDuringChargeAttempt = false;

    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();

    if (!Shotgun || !ShotgunData || ShotgunData->SecondaryChargeTime <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_ChargedShotgunSecondaryAbility::StartSecondaryCharge: Invalid Shotgun, Data, or ChargeTime. Cancelling."));
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
        return;
    }

    // Store charge timing info
    ChargeStartTime = GetWorld()->GetTimeSeconds();
    ChargeDuration = ShotgunData->SecondaryChargeTime;

    // Reset charge progress
    Shotgun->SetSecondaryChargeProgress(0.0f);

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ChargeInProgressTag.IsValid() && ASC)
    {
        ASC->AddLooseGameplayTag(ChargeInProgressTag);
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::StartSecondaryCharge: Added ChargeInProgressTag %s"), *ChargeInProgressTag.ToString());
    }
    if (ShotgunData->SecondaryChargeStartCue.IsValid() && ASC)
    {
        ASC->ExecuteGameplayCue(ShotgunData->SecondaryChargeStartCue, ASC->MakeEffectContext());
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::StartSecondaryCharge: Executed SecondaryChargeStartCue %s"), *ShotgunData->SecondaryChargeStartCue.ToString());
    }
    Shotgun->K2_OnSecondaryChargeStart();

    // Start progress update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(ChargeProgressTimerHandle, this, &US_ChargedShotgunSecondaryAbility::UpdateChargeProgress, 0.05f, true);
    }

    ChargeTimerTask = UAbilityTask_WaitDelay::WaitDelay(this, ShotgunData->SecondaryChargeTime);
    if (ChargeTimerTask)
    {
        ChargeTimerTask->OnFinish.AddDynamic(this, &US_ChargedShotgunSecondaryAbility::OnSecondaryChargeComplete);
        ChargeTimerTask->ReadyForActivation();
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::StartSecondaryCharge: Charge timer started for %f seconds."), ShotgunData->SecondaryChargeTime);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("US_ChargedShotgunSecondaryAbility::StartSecondaryCharge: Failed to create ChargeTimerTask. Cancelling."));
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
    }
}

void US_ChargedShotgunSecondaryAbility::UpdateChargeProgress()
{
    if (!bIsCharging) return;

    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    if (!Shotgun) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ElapsedTime = CurrentTime - ChargeStartTime;
    float Progress = FMath::Clamp(ElapsedTime / ChargeDuration, 0.0f, 1.0f);

    Shotgun->SetSecondaryChargeProgress(Progress);
}

void US_ChargedShotgunSecondaryAbility::OnSecondaryChargeComplete()
{
    UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::OnSecondaryChargeComplete. bIsCharging: %d, bInputReleasedDuringChargeAttempt: %d"), bIsCharging, bInputReleasedDuringChargeAttempt);
    if (!bIsCharging) return;

    // Stop progress updates
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ChargeProgressTimerHandle);
    }

    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    if (Shotgun)
    {
        Shotgun->SetSecondaryChargeProgress(1.0f); // Ensure it's at 100%
    }

    bIsCharging = false;
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ChargeInProgressTag.IsValid() && ASC)
    {
        ASC->RemoveLooseGameplayTag(ChargeInProgressTag);
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::OnSecondaryChargeComplete: Removed ChargeInProgressTag %s"), *ChargeInProgressTag.ToString());
    }

    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();

    if (bInputReleasedDuringChargeAttempt) // Input was released *during* the charge timer
    {
        if (Shotgun) Shotgun->K2_OnSecondaryChargeCancelled();
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::OnSecondaryChargeComplete: Input released during charge. Ending ability."));
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true); // true for bWasCancelled
        return;
    }

    // Charge completed successfully, input was held
    bOverchargedShotStored = true;
    if (OverchargedStateTag.IsValid() && ASC)
    {
        ASC->AddLooseGameplayTag(OverchargedStateTag);
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::OnSecondaryChargeComplete: Added OverchargedStateTag %s"), *OverchargedStateTag.ToString());
    }
    if (ShotgunData && ShotgunData->SecondaryChargeHeldCue.IsValid() && ASC)
    {
        ASC->ExecuteGameplayCue(ShotgunData->SecondaryChargeHeldCue, ASC->MakeEffectContext());
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::OnSecondaryChargeComplete: Executed SecondaryChargeHeldCue %s"), *ShotgunData->SecondaryChargeHeldCue.ToString());
    }
    if (Shotgun) Shotgun->K2_OnSecondaryChargeHeld();
    UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::OnSecondaryChargeComplete: Overcharged shot stored. Waiting for input release to fire."));
}

void US_ChargedShotgunSecondaryAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo);
    UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::InputReleased. bIsCharging: %d, bOverchargedShotStored: %d"), bIsCharging, bOverchargedShotStored);

    if (bIsCharging) // Input released while charge timer was active
    {
        bInputReleasedDuringChargeAttempt = true;
        // OnSecondaryChargeComplete will handle cancelling the ability if this flag is true
        return;
    }

    if (bOverchargedShotStored) // Input released after charge was complete and stored
    {
        if (CommitAbility(Handle, ActorInfo, ActivationInfo)) // Re-commit for the fire action
        {
            AttemptFireOverchargedShot();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("US_ChargedShotgunSecondaryAbility::InputReleased: Failed to commit for firing overcharged shot. Cancelling."));
            AS_ChargedShotgun* Shotgun = GetChargedShotgun();
            if (Shotgun) Shotgun->K2_OnSecondaryChargeCancelled();
            CancelAbility(Handle, ActorInfo, ActivationInfo, true); // true for bWasCancelled as fire didn't happen
        }
    }
    else // Input released, not charging, no shot stored (e.g. ability ended for other reasons)
    {
        if (IsActive()) // Only end if still somehow active
        {
            EndAbility(Handle, ActorInfo, ActivationInfo, false, true); // true for bWasCancelled
        }
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
        UE_LOG(LogTemp, Error, TEXT("US_ChargedShotgunSecondaryAbility::AttemptFireOverchargedShot: Precondition failed."));
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::AttemptFireOverchargedShot for %s"), *Weapon->GetName());

    Super::ApplyAmmoCost(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo());
    ApplyWeaponLockoutCooldown(); // This ability's specific cooldown

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
    else
    {
        FireStartLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        FireDirection = Weapon->GetActorForwardVector();
    }

    // The weapon itself will fetch PelletCount, Spread, Range, Damage from its DataAsset for secondary fire
    Weapon->ExecuteSecondaryFire(FireStartLocation, FireDirection, CurrentEventData ? *CurrentEventData : FGameplayEventData());

    if (WeaponData->SecondaryOverchargedFireCue.IsValid() && ASC)
    {
        ASC->ExecuteGameplayCue(WeaponData->SecondaryOverchargedFireCue, ASC->MakeEffectContext());
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::AttemptFireOverchargedShot: Executed SecondaryOverchargedFireCue %s"), *WeaponData->SecondaryOverchargedFireCue.ToString());
    }
    else if (WeaponData->MuzzleFlashCueTag.IsValid() && ASC) // Fallback to generic muzzle flash if no specific overcharge cue
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        CueParams.Normal = FireDirection;
        CueParams.Instigator = Character;
        CueParams.EffectCauser = Weapon;
        ASC->ExecuteGameplayCue(WeaponData->MuzzleFlashCueTag, CueParams);
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::AttemptFireOverchargedShot: Executed MuzzleFlashCue (fallback) %s"), *WeaponData->MuzzleFlashCueTag.ToString());
    }

    Weapon->K2_OnSecondaryChargeReleased();

    bOverchargedShotStored = false;
    if (OverchargedStateTag.IsValid() && ASC)
    {
        ASC->RemoveLooseGameplayTag(OverchargedStateTag);
        UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::AttemptFireOverchargedShot: Removed OverchargedStateTag %s"), *OverchargedStateTag.ToString());
    }

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
            UE_LOG(LogTemp, Log, TEXT("Charged Shotgun Secondary: Applied Weapon Lockout Cooldown Effect %s."), *ShotgunData->SecondaryFireLockoutEffect->GetName());
        }
    }
}

void US_ChargedShotgunSecondaryAbility::ResetAbilityState()
{
    bIsCharging = false;
    bOverchargedShotStored = false;
    bInputReleasedDuringChargeAttempt = false;
    ChargeStartTime = 0.0f;
    ChargeDuration = 0.0f;

    // Stop progress timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ChargeProgressTimerHandle);
    }

    // Reset weapon charge progress
    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    if (Shotgun)
    {
        Shotgun->SetSecondaryChargeProgress(0.0f);
    }

    if (ChargeTimerTask && ChargeTimerTask->IsActive())
    {
        ChargeTimerTask->EndTask();
    }
    ChargeTimerTask = nullptr;

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC)
    {
        if (ChargeInProgressTag.IsValid() && ASC->HasMatchingGameplayTag(ChargeInProgressTag))
        {
            ASC->RemoveLooseGameplayTag(ChargeInProgressTag);
            UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::ResetAbilityState: Removed ChargeInProgressTag %s"), *ChargeInProgressTag.ToString());
        }
        if (OverchargedStateTag.IsValid() && ASC->HasMatchingGameplayTag(OverchargedStateTag))
        {
            ASC->RemoveLooseGameplayTag(OverchargedStateTag);
            UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::ResetAbilityState: Removed OverchargedStateTag %s"), *OverchargedStateTag.ToString());
        }
    }
}

void US_ChargedShotgunSecondaryAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    UE_LOG(LogTemp, Log, TEXT("US_ChargedShotgunSecondaryAbility::EndAbility for %s. bWasCancelled: %d"), *GetNameSafe(this), bWasCancelled);
    ResetAbilityState();
    if (WaitInputReleaseTask && WaitInputReleaseTask->IsActive())
    {
        WaitInputReleaseTask->EndTask();
    }
    WaitInputReleaseTask = nullptr;
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}