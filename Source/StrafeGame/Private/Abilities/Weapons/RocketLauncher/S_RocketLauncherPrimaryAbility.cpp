#include "Abilities/Weapons/RocketLauncher/S_RocketLauncherPrimaryAbility.h"
#include "Weapons/RocketLauncher/S_RocketLauncher.h"
#include "Weapons/RocketLauncher/S_RocketLauncherDataAsset.h"
#include "Weapons/RocketLauncher/S_RocketProjectile.h"
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h" 
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameplayEffectTypes.h" 
#include "Abilities/GameplayAbility.h"

US_RocketLauncherPrimaryAbility::US_RocketLauncherPrimaryAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

AS_RocketLauncher* US_RocketLauncherPrimaryAbility::GetRocketLauncher() const
{
    return Cast<AS_RocketLauncher>(GetEquippedWeapon());
}

const US_RocketLauncherDataAsset* US_RocketLauncherPrimaryAbility::GetRocketLauncherData() const
{
    return Cast<US_RocketLauncherDataAsset>(GetEquippedWeaponData());
}

bool US_RocketLauncherPrimaryAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    const US_RocketLauncherDataAsset* RocketData = GetRocketLauncherData();
    if (!RocketData || !RocketData->ProjectileClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_RocketLauncherPrimaryAbility: No ProjectileClass defined in RocketLauncherDataAsset."));
        return false;
    }
    return true;
}

void US_RocketLauncherPrimaryAbility::PerformWeaponFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    AS_Character* Character = GetOwningSCharacter();
    AS_RocketLauncher* RocketLauncher = GetRocketLauncher();
    const US_RocketLauncherDataAsset* RocketLauncherData = GetRocketLauncherData();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    if (!Character || !RocketLauncher || !RocketLauncherData || !RocketLauncherData->ProjectileClass || !ASC)
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    FVector FireStartLocation;
    FRotator CamRot; // CORRECTED
    FVector FireDirection;
    AController* Controller = Character->GetController();
    if (Controller)
    {
        FVector MuzzleSocketLocation = RocketLauncher->GetWeaponMeshComponent()->GetSocketLocation(RocketLauncherData->MuzzleFlashSocketName); // CORRECTED

        FVector CamLoc;
        Controller->GetPlayerViewPoint(CamLoc, CamRot); // CORRECTED
        FireDirection = CamRot.Vector();

        const float MaxTraceDist = RocketLauncherData->MaxAimTraceRange > 0.f ? RocketLauncherData->MaxAimTraceRange : 100000.0f; // CORRECTED (using MaxAimTraceRange from base DA)
        FVector CamTraceEnd = CamLoc + FireDirection * MaxTraceDist;

        FHitResult CameraTraceHit;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Character);
        QueryParams.AddIgnoredActor(RocketLauncher);

        FireStartLocation = MuzzleSocketLocation;
        if (GetWorld()->LineTraceSingleByChannel(CameraTraceHit, CamLoc, CamTraceEnd, ECC_Visibility, QueryParams))
        {
            FireDirection = (CameraTraceHit.ImpactPoint - MuzzleSocketLocation).GetSafeNormal();
        }
        else {
            FireDirection = (CamTraceEnd - MuzzleSocketLocation).GetSafeNormal();
        }
    }
    else
    {
        FireStartLocation = RocketLauncher->GetWeaponMeshComponent()->GetSocketLocation(RocketLauncherData->MuzzleFlashSocketName); // CORRECTED
        FireDirection = RocketLauncher->GetActorForwardVector();
    }

    const FGameplayEventData* AbilityTriggerData = GetCurrentAbilityTriggerData(); // CORRECTED
    RocketLauncher->ExecuteFire(FireStartLocation, FireDirection, AbilityTriggerData ? *AbilityTriggerData : FGameplayEventData(), 0.f, 0.f, RocketLauncherData->ProjectileClass);

    UAbilityTask_PlayMontageAndWait* MontageTask = PlayWeaponMontage(RocketLauncherData->FireMontage);
    if (MontageTask)
    {
        MontageTask->OnCompleted.AddDynamic(this, &US_RocketLauncherPrimaryAbility::OnFireMontageCompleted);
        MontageTask->OnInterrupted.AddDynamic(this, &US_RocketLauncherPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        MontageTask->OnCancelled.AddDynamic(this, &US_RocketLauncherPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        MontageTask->ReadyForActivation();
    }
    else
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }

    if (RocketLauncherData->MuzzleFlashCueTag.IsValid())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = RocketLauncher->GetWeaponMeshComponent()->GetSocketLocation(RocketLauncherData->MuzzleFlashSocketName); // CORRECTED
        CueParams.Normal = FireDirection;
        CueParams.Instigator = Character;
        CueParams.EffectCauser = RocketLauncher;
        ASC->ExecuteGameplayCue(RocketLauncherData->MuzzleFlashCueTag, CueParams);
    }

    if (!MontageTask)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }
}

// OnFireMontageCompleted and OnFireMontageInterruptedOrCancelled are inherited from US_WeaponPrimaryAbility