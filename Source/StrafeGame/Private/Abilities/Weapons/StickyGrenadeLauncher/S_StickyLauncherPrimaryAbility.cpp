#include "Abilities/Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncherPrimaryAbility.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncher.h"
#include "Weapons/DataAssets/StickyGrenadeLauncher/S_StickyGrenadeLauncherDataAsset.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeProjectile.h"
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Controller.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

US_StickyGrenadeLauncherPrimaryAbility::US_StickyGrenadeLauncherPrimaryAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    // AbilityInputID inherited.
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

    // Check max active projectiles
    if (Launcher->GetActiveProjectiles().Num() >= LauncherData->MaxActiveProjectiles)
    {
        // UE_LOG(LogTemp, Warning, TEXT("US_StickyGrenadeLauncherPrimaryAbility: Max active stickies reached (%d)."), LauncherData->MaxActiveProjectiles);
        // Optionally, add a gameplay tag for UI feedback: "Ability.Feedback.MaxProjectilesReached"
        if (OptionalRelevantTags)
        {
            // OptionalRelevantTags->AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Feedback.MaxProjectilesReached")));
        }
        return false;
    }
    // Super::CanActivateAbility checks ammo, cooldown (0.333s rate of fire), weapon state.
    return true;
}

void US_StickyGrenadeLauncherPrimaryAbility::PerformWeaponFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    AS_Character* Character = GetOwningSCharacter();
    AS_StickyGrenadeLauncher* Launcher = GetStickyLauncher();
    const US_StickyGrenadeLauncherDataAsset* LauncherData = GetStickyLauncherData();

    if (!Character || !Launcher || !LauncherData || !LauncherData->ProjectileClass || !ActorInfo || !ActorInfo->AbilitySystemComponent.Get())
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    // 1. Get Aiming Data
    FVector FireStartLocation;
    FVector FireDirection;
    AController* Controller = Character->GetController();
    if (Controller)
    {
        FVector MuzzleSocketLocation = Launcher->GetWeaponMeshComponent()->GetSocketLocation(LauncherData->MuzzleFlashSocketName);
        FVector CamLoc, CamRotDir_Unused;
        Controller->GetPlayerViewPoint(CamLoc, CamRotDir_Unused);
        FVector CamAimDir = Controller->GetControlRotation().Vector();
        const float MaxFireDistance = 100000.0f;
        FVector CamTraceEnd = CamLoc + CamAimDir * MaxFireDistance;
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

    // 2. Play Fire Montage
    UAbilityTask_PlayMontageAndWait* MontageTask = PlayWeaponMontage(LauncherData->FireMontage);
    if (MontageTask)
    {
        MontageTask->OnCompleted.AddDynamic(this, &US_StickyGrenadeLauncherPrimaryAbility::OnFireMontageCompleted);
        MontageTask->OnInterrupted.AddDynamic(this, &US_StickyGrenadeLauncherPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        MontageTask->OnCancelled.AddDynamic(this, &US_StickyGrenadeLauncherPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        MontageTask->ReadyForActivation();
    }

    // 3. Call Weapon's ExecuteFire (which calls AS_ProjectileWeapon's implementation)
    Launcher->ExecuteFire(FireStartLocation, FireDirection, nullptr, 0.f, 0.f, LauncherData->ProjectileClass);

    // 4. Trigger Muzzle Flash Cue
    if (LauncherData->MuzzleFlashCueTag.IsValid())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = Launcher->GetWeaponMeshComponent()->GetSocketLocation(LauncherData->MuzzleFlashSocketName);
        CueParams.Normal = FireDirection;
        CueParams.Instigator = Character;
        CueParams.EffectCauser = Launcher;
        ActorInfo->AbilitySystemComponent->ExecuteGameplayCue(LauncherData->MuzzleFlashCueTag, CueParams);
    }

    if (!MontageTask) // If no montage, end immediately
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }
}