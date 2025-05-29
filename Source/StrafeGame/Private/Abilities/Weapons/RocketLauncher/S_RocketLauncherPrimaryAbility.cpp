// Source/StrafeGame/Private/Abilities/Weapons/RocketLauncher/S_RocketLauncherPrimaryAbility.cpp
#include "Abilities/Weapons/RocketLauncher/S_RocketLauncherPrimaryAbility.h"
#include "Weapons/RocketLauncher/S_RocketLauncher.h"
#include "Weapons/RocketLauncher/S_RocketLauncherDataAsset.h"
#include "Weapons/RocketLauncher/S_RocketProjectile.h" // Should be included if DA might not have it
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
    if (!RocketData || !RocketData->ProjectileClass) // Check if the DataAsset specifies a projectile
    {
        UE_LOG(LogTemp, Warning, TEXT("US_RocketLauncherPrimaryAbility::CanActivateAbility: No ProjectileClass defined in RocketLauncherDataAsset."));
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
        UE_LOG(LogTemp, Error, TEXT("US_RocketLauncherPrimaryAbility::PerformWeaponFire: Precondition failed."));
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("US_RocketLauncherPrimaryAbility::PerformWeaponFire for %s"), *RocketLauncher->GetName());

    FVector FireStartLocation;
    FRotator CamRot;
    FVector FireDirection;
    AController* Controller = Character->GetController();
    if (Controller)
    {
        FVector MuzzleSocketLocation = RocketLauncher->GetWeaponMeshComponent()->GetSocketLocation(RocketLauncherData->MuzzleFlashSocketName);
        FVector CamLoc;
        Controller->GetPlayerViewPoint(CamLoc, CamRot);
        FireDirection = CamRot.Vector();

        const float MaxTraceDist = RocketLauncherData->MaxAimTraceRange > 0.f ? RocketLauncherData->MaxAimTraceRange : 100000.0f;
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
        FireStartLocation = RocketLauncher->GetWeaponMeshComponent()->GetSocketLocation(RocketLauncherData->MuzzleFlashSocketName);
        FireDirection = RocketLauncher->GetActorForwardVector();
    }

    // Weapon actor (AS_RocketLauncher) will use its own DataAsset to get ProjectileClass, LaunchSpeed, etc.
    RocketLauncher->ExecutePrimaryFire(FireStartLocation, FireDirection, CurrentEventData ? *CurrentEventData : FGameplayEventData());

    FireMontageTask = PlayWeaponMontage(RocketLauncherData->FireMontage); // Corrected: Use FireMontageTask member
    if (FireMontageTask)
    {
        FireMontageTask->OnCompleted.AddDynamic(this, &US_RocketLauncherPrimaryAbility::OnFireMontageCompleted);
        FireMontageTask->OnInterrupted.AddDynamic(this, &US_RocketLauncherPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        FireMontageTask->OnCancelled.AddDynamic(this, &US_RocketLauncherPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        FireMontageTask->ReadyForActivation();
        UE_LOG(LogTemp, Log, TEXT("US_RocketLauncherPrimaryAbility::PerformWeaponFire: Fire montage started."));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("US_RocketLauncherPrimaryAbility::PerformWeaponFire: No fire montage. Ending ability."));
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false); // End immediately if no montage
    }

    if (RocketLauncherData->MuzzleFlashCueTag.IsValid())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = RocketLauncher->GetWeaponMeshComponent()->GetSocketLocation(RocketLauncherData->MuzzleFlashSocketName);
        CueParams.Normal = FireDirection;
        CueParams.Instigator = Character;
        CueParams.EffectCauser = RocketLauncher;
        ASC->ExecuteGameplayCue(RocketLauncherData->MuzzleFlashCueTag, CueParams);
        UE_LOG(LogTemp, Log, TEXT("US_RocketLauncherPrimaryAbility::PerformWeaponFire: Executed MuzzleFlashCue %s"), *RocketLauncherData->MuzzleFlashCueTag.ToString());
    }

    // Do not end ability here if montage is playing. OnFireMontageCompleted/Cancelled will handle it.
    if (!FireMontageTask) // Double check, if task failed to activate or was null
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }
}

// OnFireMontageCompleted and OnFireMontageInterruptedOrCancelled are inherited from US_WeaponPrimaryAbility
// and will call EndAbility.