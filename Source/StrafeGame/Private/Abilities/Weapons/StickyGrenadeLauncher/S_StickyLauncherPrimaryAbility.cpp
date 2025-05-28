#include "Abilities/Weapons/StickyGrenadeLauncher/S_StickyLauncherPrimaryAbility.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncher.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncherDataAsset.h" // Correct path for specific DA
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeProjectile.h"
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Controller.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameplayEffectTypes.h" 

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
        return false;
    }

    if (Launcher->GetActiveProjectiles().Num() >= LauncherData->MaxActiveProjectiles)
    {
        if (OptionalRelevantTags)
        {
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
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    FVector FireStartLocation;
    FRotator CamRot; // CORRECTED
    FVector FireDirection;
    AController* Controller = Character->GetController();
    if (Controller)
    {
        FVector MuzzleSocketLocation = Launcher->GetWeaponMeshComponent()->GetSocketLocation(LauncherData->MuzzleFlashSocketName); // CORRECTED
        FVector CamLoc;
        Controller->GetPlayerViewPoint(CamLoc, CamRot); // CORRECTED
        FireDirection = CamRot.Vector();

        const float MaxTraceDist = LauncherData->MaxAimTraceRange > 0.f ? LauncherData->MaxAimTraceRange : 100000.0f; // CORRECTED
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
        FireStartLocation = Launcher->GetWeaponMeshComponent()->GetSocketLocation(LauncherData->MuzzleFlashSocketName); // CORRECTED
        FireDirection = Launcher->GetActorForwardVector();
    }

    const FGameplayEventData* AbilityTriggerData = GetCurrentAbilityTriggerData(); // CORRECTED
    Launcher->ExecuteFire(FireStartLocation, FireDirection, AbilityTriggerData ? *AbilityTriggerData : FGameplayEventData(), 0.f, 0.f, LauncherData->ProjectileClass);

    UAbilityTask_PlayMontageAndWait* MontageTask = PlayWeaponMontage(LauncherData->FireMontage);
    if (MontageTask)
    {
        MontageTask->OnCompleted.AddDynamic(this, &US_StickyGrenadeLauncherPrimaryAbility::OnFireMontageCompleted);
        MontageTask->OnInterrupted.AddDynamic(this, &US_StickyGrenadeLauncherPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        MontageTask->OnCancelled.AddDynamic(this, &US_StickyGrenadeLauncherPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        MontageTask->ReadyForActivation();
    }
    else
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }

    if (LauncherData->MuzzleFlashCueTag.IsValid())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = Launcher->GetWeaponMeshComponent()->GetSocketLocation(LauncherData->MuzzleFlashSocketName); // CORRECTED
        CueParams.Normal = FireDirection;
        CueParams.Instigator = Character;
        CueParams.EffectCauser = Launcher;
        ASC->ExecuteGameplayCue(LauncherData->MuzzleFlashCueTag, CueParams);
    }

    if (!MontageTask)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }
}

// OnFireMontageCompleted and OnFireMontageInterruptedOrCancelled are inherited.