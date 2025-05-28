#include "Abilities/Weapons/ChargedShotgun/S_ChargedShotgunPrimaryAbility.h"
#include "Weapons/ChargedShotgun/S_ChargedShotgun.h"
#include "Weapons/ChargedShotgun/S_ChargedShotgunDataAsset.h" // You'll need to create this
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Controller.h"

US_ChargedShotgunPrimaryAbility::US_ChargedShotgunPrimaryAbility()
{
    // Instance per actor because it has state (isCharging)
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted; // Good for responsive feel

    // Define relevant tags - these should match tags your GEs use or apply
    ChargeInProgressTag = FGameplayTag::RequestGameplayTag(FName("State.Weapon.Charging.Primary.Shotgun"));
    // EarlyReleaseCooldownTag is the tag that the actual Cooldown GE will have in its GrantedTags.
    // This ability might check for it in CanActivate, but the main blocking is done by the ASC.

    bIsCharging = false;
    bFiredDuringChargeLoop = false;

    // This ability is activated by player input, so AbilityInputID is important.
    // It will be set by AS_Character::HandleWeaponEquipped based on WeaponDataAsset.
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
    // Add any specific checks for ChargedShotgunPrimary, e.g., not already charging secondary
    // Or if the "SecondaryLockoutCooldown" is active.
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();
    if (ShotgunData && ShotgunData->SecondaryFireLockoutTag.IsValid()) // Assuming SecondaryFireLockoutTag is on US_ChargedShotgunDataAsset
    {
        if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(ShotgunData->SecondaryFireLockoutTag))
        {
            return false; // Cannot activate if locked out by secondary fire
        }
    }
    return true;
}

void US_ChargedShotgunPrimaryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData); // Handles CommitAbility internally

    if (!HasEnoughResourcesToActivate()) // From CommitAbility, or re-check ammo specifically
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    ResetAbilityState();

    // Start waiting for input release immediately
    WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true); // true to end ability if released and not overridden
    if (WaitInputReleaseTask)
    {
        // We don't bind to OnRelease here directly because InputReleased is an override
        WaitInputReleaseTask->ReadyForActivation();
    }
    else
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    StartCharge();
}

void US_ChargedShotgunPrimaryAbility::StartCharge()
{
    if (bIsCharging) return;

    bIsCharging = true;
    bFiredDuringChargeLoop = false; // Reset this flag at the start of a new charge cycle

    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();

    if (!Shotgun || !ShotgunData || ShotgunData->PrimaryChargeTime <= 0.f)
    {
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
        return;
    }

    if (ChargeInProgressTag.IsValid())
    {
        GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(ChargeInProgressTag);
    }
    if (ShotgunData->PrimaryChargeStartCue.IsValid()) // Assuming these Cue tags exist on US_ChargedShotgunDataAsset
    {
        GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(ShotgunData->PrimaryChargeStartCue, GetAbilitySystemComponentFromActorInfo()->MakeEffectContext());
    }
    Shotgun->K2_OnPrimaryChargeStart();


    ChargeTimerTask = UAbilityTask_WaitDelay::WaitDelay(this, ShotgunData->PrimaryChargeTime);
    if (ChargeTimerTask)
    {
        ChargeTimerTask->OnFinish.AddDynamic(this, &US_ChargedShotgunPrimaryAbility::OnChargeComplete);
        ChargeTimerTask->ReadyForActivation();
    }
    else
    {
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
    }
}

void US_ChargedShotgunPrimaryAbility::OnChargeComplete()
{
    if (!bIsCharging) // Could have been cancelled by input release
    {
        return;
    }

    bIsCharging = false; // Charge is complete
    if (ChargeInProgressTag.IsValid())
    {
        GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(ChargeInProgressTag);
    }

    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    if (Shotgun) Shotgun->K2_OnPrimaryChargeComplete();

    DoFire();
    bFiredDuringChargeLoop = true; // Mark that we fired in this activation

    // If input is still held (WaitInputReleaseTask is still active), restart charge
    if (WaitInputReleaseTask && WaitInputReleaseTask->IsActive() && IsInputPressed()) // IsInputPressed() is a UGameplayAbility helper
    {
        // Check ammo again before restarting charge
        if (CanActivateAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), nullptr, nullptr, nullptr))
        {
            StartCharge(); // Loop
        }
        else
        {
            EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
        }
    }
    else // Input was released or task ended
    {
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
    }
}

void US_ChargedShotgunPrimaryAbility::DoFire()
{
    // Standard fire logic from US_WeaponPrimaryAbility::PerformWeaponFire (or adapted here)
    // Applies ammo cost, plays montage, calls weapon->ExecuteFire, applies cooldown.

    AS_Character* Character = GetOwningSCharacter();
    AS_ChargedShotgun* Weapon = GetChargedShotgun();
    const US_ChargedShotgunDataAsset* WeaponData = GetChargedShotgunData(); // This is US_ChargedShotgunDataAsset now

    if (!Character || !Weapon || !WeaponData || !GetAbilityActorInfo())
    {
        return;
    }

    // 1. Apply Ammo Cost (using the specific primary fire cost from WeaponData)
    Super::ApplyAmmoCost(GetCurrentAbilitySpecHandle(), GetAbilityActorInfo(), GetCurrentActivationInfo()); // Super uses WeaponData->AmmoCostEffect_Primary

    // 2. Apply Cooldown (using the specific primary fire cooldown from WeaponData)
    Super::ApplyAbilityCooldown(GetCurrentAbilitySpecHandle(), GetAbilityActorInfo(), GetCurrentActivationInfo()); // Super uses WeaponData->CooldownTag_Primary

    // 3. Get Aiming Data
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
        const float MaxFireDistance = WeaponData->PrimaryFireHitscanRange; // Use specific range
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
    else
    {
        FireStartLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        FireDirection = Weapon->GetActorForwardVector();
    }

    // 4. Play Fire Montage
    // The base US_WeaponPrimaryAbility::PerformWeaponFire has montage logic, we can replicate or call it.
    // For now, let's assume montage handling is simple or part of a gameplay cue.
    // If this ability plays its own montage for firing after charge:
    // UAbilityTask_PlayMontageAndWait* FireTask = PlayWeaponMontage(WeaponData->FireMontage1P, WeaponData->FireMontage3P);
    // if(FireTask) { FireTask->ReadyForActivation(); /* Bind delegates if needed */ }

    // 5. Call Weapon's ExecuteFire
    // Parameters like pellet count, spread, range come from US_ChargedShotgunDataAsset
    Weapon->ExecuteFire(FireStartLocation, FireDirection, nullptr, WeaponData->PrimaryFireSpreadAngle, WeaponData->PrimaryFireHitscanRange, nullptr);

    // 6. Muzzle Flash Cue
    if (WeaponData->MuzzleFlashCueTag.IsValid() && GetAbilityActorInfo()->AbilitySystemComponent.IsValid())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        CueParams.Normal = FireDirection;
        CueParams.Instigator = Character;
        CueParams.EffectCauser = Weapon;
        GetAbilityActorInfo()->AbilitySystemComponent->ExecuteGameplayCue(WeaponData->MuzzleFlashCueTag, CueParams);
    }

    UE_LOG(LogTemp, Log, TEXT("Charged Shotgun Primary Fired"));
}

void US_ChargedShotgunPrimaryAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo); // Important to call super if it does anything

    if (bIsCharging) // If input released *while* charging
    {
        ApplyEarlyReleasePenalty();
        AS_ChargedShotgun* Shotgun = GetChargedShotgun();
        if (Shotgun) Shotgun->K2_OnPrimaryChargeCancelled();
    }
    // If not charging (e.g., input released after an auto-fire but before next charge started),
    // or if charge completed and fired, the EndAbility in OnChargeComplete or the task itself handles it.
    // The WaitInputRelease task with bEndAbilityWhenReleased=true would also call EndAbility.
    // We need to ensure EndAbility is called cleanly.
    if (IsActive()) // If ability hasn't already ended
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, bIsCharging); // bWasCancelled is true if we were charging
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
            UE_LOG(LogTemp, Log, TEXT("Charged Shotgun Primary: Applied Early Release Penalty Cooldown."));
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

    if (ChargeInProgressTag.IsValid() && GetAbilitySystemComponentFromActorInfo())
    {
        GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(ChargeInProgressTag);
    }
}

void US_ChargedShotgunPrimaryAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    ResetAbilityState();
    if (WaitInputReleaseTask && WaitInputReleaseTask->IsActive())
    {
        WaitInputReleaseTask->EndTask(); // Important to clean up the input task
    }
    WaitInputReleaseTask = nullptr;
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}