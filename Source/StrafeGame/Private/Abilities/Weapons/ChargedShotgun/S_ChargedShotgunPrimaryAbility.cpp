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

    ChargeInProgressTag = FGameplayTag::RequestGameplayTag(FName("State.Weapon.Charging.Primary.Shotgun"));

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
            return false;
        }
    }
    return true;
}

void US_ChargedShotgunPrimaryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

    StartCharge();
}

void US_ChargedShotgunPrimaryAbility::StartCharge()
{
    if (bIsCharging) return;

    bIsCharging = true;
    bFiredDuringChargeLoop = false;

    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    const US_ChargedShotgunDataAsset* ShotgunData = GetChargedShotgunData();

    if (!Shotgun || !ShotgunData || ShotgunData->PrimaryChargeTime <= 0.f)
    {
        CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
        return;
    }

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ChargeInProgressTag.IsValid() && ASC)
    {
        ASC->AddLooseGameplayTag(ChargeInProgressTag);
    }
    if (ShotgunData->PrimaryChargeStartCue.IsValid() && ASC)
    {
        ASC->ExecuteGameplayCue(ShotgunData->PrimaryChargeStartCue, ASC->MakeEffectContext());
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
    if (!bIsCharging)
    {
        return;
    }

    bIsCharging = false;
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ChargeInProgressTag.IsValid() && ASC)
    {
        ASC->RemoveLooseGameplayTag(ChargeInProgressTag);
    }

    AS_ChargedShotgun* Shotgun = GetChargedShotgun();
    if (Shotgun) Shotgun->K2_OnPrimaryChargeComplete();

    if (CommitAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo()))
    {
        DoFire();
        bFiredDuringChargeLoop = true;
    }
    else
    {
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
        return;
    }

    if (WaitInputReleaseTask && WaitInputReleaseTask->IsActive() && IsInputPressed())
    {
        if (CanActivateAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), nullptr, nullptr, nullptr))
        {
            StartCharge();
        }
        else
        {
            EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
        }
    }
    else
    {
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
    }
}

void US_ChargedShotgunPrimaryAbility::DoFire()
{
    AS_Character* Character = GetOwningSCharacter();
    AS_ChargedShotgun* Weapon = GetChargedShotgun();
    const US_ChargedShotgunDataAsset* WeaponData = GetChargedShotgunData();

    if (!Character || !Weapon || !WeaponData || !GetCurrentActorInfo())
    {
        return;
    }

    Super::ApplyAmmoCost(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo());
    Super::ApplyAbilityCooldown(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo());

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
        FireStartLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); // CORRECTED
        FireDirection = Weapon->GetActorForwardVector();
    }

    const FGameplayEventData* AbilityTriggerData = CurrentEventData;
    Weapon->ExecuteFire(FireStartLocation, FireDirection, AbilityTriggerData ? *AbilityTriggerData : FGameplayEventData(), WeaponData->PrimaryFireSpreadAngle, WeaponData->PrimaryFireHitscanRange, nullptr);

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (WeaponData->MuzzleFlashCueTag.IsValid() && ASC) // CORRECTED
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); // CORRECTED
        CueParams.Normal = FireDirection;
        CueParams.Instigator = Character;
        CueParams.EffectCauser = Weapon;
        ASC->ExecuteGameplayCue(WeaponData->MuzzleFlashCueTag, CueParams); // CORRECTED
    }

    if (WeaponData->PrimaryChargeCompleteCue.IsValid() && ASC) // CORRECTED
    {
        ASC->ExecuteGameplayCue(WeaponData->PrimaryChargeCompleteCue, ASC->MakeEffectContext()); // CORRECTED
    }

    UE_LOG(LogTemp, Log, TEXT("Charged Shotgun Primary Fired"));
}

void US_ChargedShotgunPrimaryAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo);

    if (bIsCharging)
    {
        ApplyEarlyReleasePenalty();
        AS_ChargedShotgun* Shotgun = GetChargedShotgun();
        if (Shotgun) Shotgun->K2_OnPrimaryChargeCancelled();
    }

    if (IsActive())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, bIsCharging);
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

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ChargeInProgressTag.IsValid() && ASC)
    {
        ASC->RemoveLooseGameplayTag(ChargeInProgressTag);
    }
}

void US_ChargedShotgunPrimaryAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    ResetAbilityState();
    if (WaitInputReleaseTask && WaitInputReleaseTask->IsActive())
    {
        WaitInputReleaseTask->EndTask();
    }
    WaitInputReleaseTask = nullptr;
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}