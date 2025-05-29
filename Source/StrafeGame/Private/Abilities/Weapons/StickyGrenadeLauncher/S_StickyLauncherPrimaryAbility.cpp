// Source/StrafeGame/Private/Abilities/Weapons/StickyGrenadeLauncher/S_StickyLauncherPrimaryAbility.cpp
#include "Abilities/Weapons/StickyGrenadeLauncher/S_StickyLauncherPrimaryAbility.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncher.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncherDataAsset.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeProjectile.h" // Should be included if DA might not have it
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Controller.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbility.h"

US_StickyGrenadeLauncherPrimaryAbility::US_StickyGrenadeLauncherPrimaryAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

AS_StickyGrenadeLauncher* US_StickyGrenadeLauncherPrimaryAbility::GetStickyLauncher() const
{
    return Cast<AS_StickyGrenadeLauncher>(GetEquippedWeapon());
}

const US_StickyGrenadeLauncherDataAsset* US_StickyGrenadeLauncherPrimaryAbility::GetStickyLauncherData() const
{
    return Cast<US_StickyGrenadeLauncherDataAsset>(GetEquippedWeaponData());
}

bool US_StickyGrenadeLauncherPrimaryAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    const AS_StickyGrenadeLauncher* Launcher = GetStickyLauncher();
    const US_StickyGrenadeLauncherDataAsset* LauncherData = GetStickyLauncherData();

    if (!Launcher || !LauncherData || !LauncherData->ProjectileClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_StickyGrenadeLauncherPrimaryAbility::CanActivateAbility: Invalid Launcher, Data, or ProjectileClass."));
        return false;
    }

    if (Launcher->GetActiveProjectiles().Num() >= LauncherData->MaxActiveProjectiles)
    {
        UE_LOG(LogTemp, Log, TEXT("US_StickyGrenadeLauncherPrimaryAbility::CanActivateAbility: Max active projectiles (%d) reached for %s."), LauncherData->MaxActiveProjectiles, *Launcher->GetName());
        if (OptionalRelevantTags)
        {
            // Example: Add a tag to inform UI or other systems. Ensure this tag exists in your project.
            // OptionalRelevantTags->AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Feedback.MaxProjectilesReached")));
        }
        return false;
    }
    return true;
}

void US_StickyGrenadeLauncherPrimaryAbility::PerformWeaponFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    AS_Character* Character = GetOwningSCharacter();
    AS_StickyGrenadeLauncher* Launcher = GetStickyLauncher();
    const US_StickyGrenadeLauncherDataAsset* LauncherData = GetStickyLauncherData();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    if (!Character || !Launcher || !LauncherData || !LauncherData->ProjectileClass || !ASC)
    {
        UE_LOG(LogTemp, Error, TEXT("US_StickyGrenadeLauncherPrimaryAbility::PerformWeaponFire: Precondition failed."));
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("US_StickyGrenadeLauncherPrimaryAbility::PerformWeaponFire for %s"), *Launcher->GetName());

    FVector FireStartLocation;
    FRotator CamRot;
    FVector FireDirection;
    AController* Controller = Character->GetController();
    if (Controller)
    {
        FVector MuzzleSocketLocation = Launcher->GetWeaponMeshComponent()->GetSocketLocation(LauncherData->MuzzleFlashSocketName);
        FVector CamLoc;
        Controller->GetPlayerViewPoint(CamLoc, CamRot);
        FireDirection = CamRot.Vector();

        const float MaxTraceDist = LauncherData->MaxAimTraceRange > 0.f ? LauncherData->MaxAimTraceRange : 100000.0f;
        FVector CamTraceEnd = CamLoc + FireDirection * MaxTraceDist;
        FHitResult CameraTraceHit;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Character);
        QueryParams.AddIgnoredActor(Launcher);
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
        FireStartLocation = Launcher->GetWeaponMeshComponent()->GetSocketLocation(LauncherData->MuzzleFlashSocketName);
        FireDirection = Launcher->GetActorForwardVector();
    }

    // Weapon actor (AS_StickyGrenadeLauncher) will use its DataAsset to get ProjectileClass, LaunchSpeed, etc.
    Launcher->ExecutePrimaryFire(FireStartLocation, FireDirection, CurrentEventData ? *CurrentEventData : FGameplayEventData());

    FireMontageTask = PlayWeaponMontage(LauncherData->FireMontage); // Corrected: Use FireMontageTask member
    if (FireMontageTask)
    {
        FireMontageTask->OnCompleted.AddDynamic(this, &US_StickyGrenadeLauncherPrimaryAbility::OnFireMontageCompleted);
        FireMontageTask->OnInterrupted.AddDynamic(this, &US_StickyGrenadeLauncherPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        FireMontageTask->OnCancelled.AddDynamic(this, &US_StickyGrenadeLauncherPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        FireMontageTask->ReadyForActivation();
        UE_LOG(LogTemp, Log, TEXT("US_StickyGrenadeLauncherPrimaryAbility::PerformWeaponFire: Fire montage started."));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("US_StickyGrenadeLauncherPrimaryAbility::PerformWeaponFire: No fire montage. Ending ability."));
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }

    if (LauncherData->MuzzleFlashCueTag.IsValid())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = Launcher->GetWeaponMeshComponent()->GetSocketLocation(LauncherData->MuzzleFlashSocketName);
        CueParams.Normal = FireDirection;
        CueParams.Instigator = Character;
        CueParams.EffectCauser = Launcher;
        ASC->ExecuteGameplayCue(LauncherData->MuzzleFlashCueTag, CueParams);
        UE_LOG(LogTemp, Log, TEXT("US_StickyGrenadeLauncherPrimaryAbility::PerformWeaponFire: Executed MuzzleFlashCue %s"), *LauncherData->MuzzleFlashCueTag.ToString());
    }

    if (!FireMontageTask)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }
}

// OnFireMontageCompleted and OnFireMontageInterruptedOrCancelled are inherited from US_WeaponPrimaryAbility.