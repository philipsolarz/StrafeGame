// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/Weapons/ChargedShotgun/S_ChargedShotgunPrimaryAbility.h"
#include "Weapons/ChargedShotgun/S_ChargedShotgun.h" //
#include "Weapons/ChargedShotgun/S_ChargedShotgunDataAsset.h" //
#include "Player/S_Character.h" //
#include "AbilitySystemComponent.h" //
#include "Abilities/Tasks/AbilityTask_WaitDelay.h" //
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h" //
#include "GameplayTagContainer.h" //
#include "GameFramework/Controller.h" //
#include "GameplayEffectTypes.h"  //
#include "Abilities/GameplayAbility.h" //

US_ChargedShotgunPrimaryAbility::US_ChargedShotgunPrimaryAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; //
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted; //

    ChargeInProgressTag = FGameplayTag::RequestGameplayTag(FName("State.Weapon.ChargedShotgun.PrimaryAbility.Charging")); //
    // EarlyReleaseCooldownTag is not initialized here, assuming it's used if PrimaryEarlyReleaseCooldownEffect applies a tag.

    bIsCharging = false; //
    bFiredDuringChargeLoop = false; //
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
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData(); //
    if (ShotgunData && ShotgunData->SecondaryFireLockoutTag.IsValid()) //
    {
        if (ActorInfo->AbilitySystemComponent.IsValid() && ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(ShotgunData->SecondaryFireLockoutTag)) //
        {
            return false; // Cannot activate primary if secondary lockout is active
        }
    }
    // Add ammo checks etc. from Super or specific to this ability if needed
    return true;
}

void US_ChargedShotgunPrimaryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData); //

}

void US_ChargedShotgunPrimaryAbility::StartCharge()
{
    if (bIsCharging) return; // Already charging

    bIsCharging = true; //
    bFiredDuringChargeLoop = false; // Reset this flag for a new charge cycle

    AS_ChargedShotgun* Shotgun = GetChargedShotgun(); //
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData(); //

    if (!Shotgun || !ShotgunData || ShotgunData->PrimaryChargeTime <= 0.f) //
    {
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true); //
        return;
    }

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo(); //
    if (ChargeInProgressTag.IsValid() && ASC) //
    {
        ASC->AddLooseGameplayTag(ChargeInProgressTag); //
    }
    if (ShotgunData->PrimaryChargeStartCue.IsValid() && ASC) //
    {
        ASC->ExecuteGameplayCue(ShotgunData->PrimaryChargeStartCue, ASC->MakeEffectContext()); //
    }
    Shotgun->K2_OnPrimaryChargeStart(); // Notify Blueprint

    // Start the charge timer
    ChargeTimerTask = UAbilityTask_WaitDelay::WaitDelay(this, ShotgunData->PrimaryChargeTime); //
    if (ChargeTimerTask)
    {
        ChargeTimerTask->OnFinish.AddDynamic(this, &US_ChargedShotgunPrimaryAbility::OnChargeComplete); //
        ChargeTimerTask->ReadyForActivation(); //
    }
    else
    {
        // Failed to create charge timer task
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true); //
    }
}

void US_ChargedShotgunPrimaryAbility::OnChargeComplete()
{
    if (!bIsCharging) // Could have been cancelled if input was released
    {
        return;
    }

    bIsCharging = false; // Charge is complete
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo(); //
    if (ChargeInProgressTag.IsValid() && ASC) //
    {
        ASC->RemoveLooseGameplayTag(ChargeInProgressTag); //
    }

    AS_ChargedShotgun* Shotgun = GetChargedShotgun(); //
    if (Shotgun) Shotgun->K2_OnPrimaryChargeComplete(); // Notify Blueprint

    // Attempt to commit and fire
    if (CommitAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo())) //
    {
        DoFire(); // Fire the weapon
        bFiredDuringChargeLoop = true; // Mark that we fired this iteration
    }
    else
    {
        // Failed to commit (e.g., out of mana after charge but before fire, or other blocking tags)
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true); //
        return;
    }

    // --- Corrected Input Check Logic ---
    bool bIsInputStillPressed = false;
    const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
    if (ActorInfo && ActorInfo->IsLocallyControlled())
    {
        if (const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
        {
            bIsInputStillPressed = Spec->InputPressed;
        }
    }
    // --- End Corrected Input Check Logic ---

    if (WaitInputReleaseTask && WaitInputReleaseTask->IsActive() && bIsInputStillPressed) // // Modified logic
    {
        // Check CanActivate again before restarting charge (e.g. for ammo)
        if (CanActivateAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), nullptr, nullptr, nullptr)) //
        {
            StartCharge(); // Auto-restart charge if input is held
        }
        else
        {
            // Cannot restart charge (e.g. out of ammo), so end the ability.
            EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false); //
        }
    }
    else
    {
        // Input was released during or after firing, or task is no longer active.
        // InputReleased will handle the EndAbility if it was released.
        // If input is not pressed, the ability effectively ends its active loop.
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false); //
    }
}

void US_ChargedShotgunPrimaryAbility::DoFire()
{
    AS_Character* Character = GetOwningSCharacter(); //
    AS_ChargedShotgun* Weapon = GetChargedShotgun(); //
    const US_ChargedShotgunDataAsset* WeaponData = GetChargedShotgunData(); //
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo(); // Get ASC here

    if (!Character || !Weapon || !WeaponData || !ASC) // Check ASC
    {
        return; // Should not happen if CanActivate and Commit passed
    }

    // Apply costs & cooldowns BEFORE firing
    Super::ApplyAmmoCost(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo()); //
    Super::ApplyAbilityCooldown(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo()); //

    // Determine fire direction (same logic as base S_WeaponPrimaryAbility)
    FVector FireStartLocation; //
    FRotator CamRot; //
    FVector FireDirection; //
    AController* Controller = Character->GetController(); //
    if (Controller)
    {
        FVector MuzzleLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); //
        FVector CamLoc; //
        Controller->GetPlayerViewPoint(CamLoc, CamRot); //
        FireDirection = CamRot.Vector(); //

        const float MaxFireDistance = WeaponData->PrimaryFireHitscanRange; // Use specific primary fire range
        FVector CamTraceEnd = CamLoc + FireDirection * MaxFireDistance; //
        FHitResult CameraTraceHit; //
        FCollisionQueryParams QueryParams; //
        QueryParams.AddIgnoredActor(Character); //
        QueryParams.AddIgnoredActor(Weapon); //
        FireStartLocation = MuzzleLocation; //
        if (GetWorld()->LineTraceSingleByChannel(CameraTraceHit, CamLoc, CamTraceEnd, ECC_Visibility, QueryParams)) //
        {
            FireDirection = (CameraTraceHit.ImpactPoint - MuzzleLocation).GetSafeNormal(); //
        }
        else {
            FireDirection = (CamTraceEnd - MuzzleLocation).GetSafeNormal(); //
        }
    }
    else
    {
        FireStartLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); //
        FireDirection = Weapon->GetActorForwardVector(); //
    }

    const FGameplayEventData* AbilityTriggerData = CurrentEventData; // Use the stored event data

    // Call weapon's ExecuteFire with specific primary fire parameters
    Weapon->ExecuteFire( //
        FireStartLocation, //
        FireDirection, //
        AbilityTriggerData ? *AbilityTriggerData : FGameplayEventData(), //
        WeaponData->PrimaryFireSpreadAngle, //
        WeaponData->PrimaryFireHitscanRange, //
        nullptr // No projectile for hitscan
    );

    // Play Muzzle Flash Cue
    if (WeaponData->MuzzleFlashCueTag.IsValid()) //
    {
        FGameplayCueParameters CueParams; //
        CueParams.Location = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); //
        CueParams.Normal = FireDirection; //
        CueParams.Instigator = Character; //
        CueParams.EffectCauser = Weapon; //
        ASC->ExecuteGameplayCue(WeaponData->MuzzleFlashCueTag, CueParams); //
    }

    // Play Primary Charge Complete/Fire Cue
    if (WeaponData->PrimaryChargeCompleteCue.IsValid()) //
    {
        ASC->ExecuteGameplayCue(WeaponData->PrimaryChargeCompleteCue, ASC->MakeEffectContext()); //
    }

    UE_LOG(LogTemp, Log, TEXT("Charged Shotgun Primary Fired")); //
}

void US_ChargedShotgunPrimaryAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo); // Call base implementation

    if (bIsCharging) // If input released DURING the charge timer
    {
        ApplyEarlyReleasePenalty(); //
        AS_ChargedShotgun* Shotgun = GetChargedShotgun(); //
        if (Shotgun) Shotgun->K2_OnPrimaryChargeCancelled(); // Notify BP
    }
    // else: if !bIsCharging, it means either charge completed and fired, or was already not charging.
    // The OnChargeComplete logic handles re-charge or ending if input is still pressed after a fire.
    // If input is released after a fire (and bFiredDuringChargeLoop is true), this ensures the ability ends.

    if (IsActive()) // Check if the ability is still active
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, bIsCharging); // bWasCancelled is true if released during charge
    }
}

void US_ChargedShotgunPrimaryAbility::PerformWeaponFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{

    ResetAbilityState(); // Clear flags

    // Start listening for input release immediately. The ability ends if input is released before full charge.
    WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true); //
    if (WaitInputReleaseTask)
    {
        // WaitInputReleaseTask->OnRelease.AddDynamic(this, &US_ChargedShotgunPrimaryAbility::OnInputReleased); // Bind to the correct delegate
        WaitInputReleaseTask->ReadyForActivation(); //
    }
    else
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true); //
        return;
    }

    StartCharge(); // Begin the charge process
}


void US_ChargedShotgunPrimaryAbility::ApplyEarlyReleasePenalty()
{
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData(); //
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo(); //

    if (ShotgunData && ShotgunData->PrimaryEarlyReleaseCooldownEffect && ASC) //
    {
        FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext(); //
        ContextHandle.AddSourceObject(this); //
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ShotgunData->PrimaryEarlyReleaseCooldownEffect, GetAbilityLevel(), ContextHandle); //
        if (SpecHandle.IsValid()) //
        {
            ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), SpecHandle); //
            UE_LOG(LogTemp, Log, TEXT("Charged Shotgun Primary: Applied Early Release Penalty Cooldown.")); //
        }
    }
}

void US_ChargedShotgunPrimaryAbility::ResetAbilityState()
{
    bIsCharging = false; //
    bFiredDuringChargeLoop = false; //

    // Cancel the charge timer if it's active
    if (ChargeTimerTask && ChargeTimerTask->IsActive()) //
    {
        ChargeTimerTask->EndTask(); //
    }
    ChargeTimerTask = nullptr; //

    // Remove gameplay tags
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo(); //
    if (ChargeInProgressTag.IsValid() && ASC) //
    {
        ASC->RemoveLooseGameplayTag(ChargeInProgressTag); //
    }
}

void US_ChargedShotgunPrimaryAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    ResetAbilityState(); // Ensure all state is cleared

    // Clean up WaitInputReleaseTask if it's still active
    if (WaitInputReleaseTask && WaitInputReleaseTask->IsActive()) //
    {
        WaitInputReleaseTask->EndTask(); //
    }
    WaitInputReleaseTask = nullptr; //

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled); //
}