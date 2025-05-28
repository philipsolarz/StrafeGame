#include "Abilities/Weapons/RocketLauncher/S_RocketLauncherPrimaryAbility.h"
#include "Weapons/RocketLauncher/S_RocketLauncher.h"
#include "Weapons/RocketLauncher/S_RocketLauncherDataAsset.h"
#include "Weapons/RocketLauncher/S_RocketProjectile.h"
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h" // For sound/effects if not fully cue driven
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

US_RocketLauncherPrimaryAbility::US_RocketLauncherPrimaryAbility()
{
    // This ability is likely instanced if it has tasks or complex state,
    // but for a simple fire-and-forget, NonInstanced might also work if state is managed carefully.
    // Let's stick to InstancedPerActor as per base US_WeaponAbility.
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    // AbilityInputID is inherited and should be set by AS_Character via WeaponDataAsset.

    // The "spam" firing (hold to fire) is handled by the standard GameplayAbility activation flow.
    // If the input is held and the cooldown (from rate of fire) has elapsed, the ability can be activated again.
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
    if (!RocketData || !RocketData->ProjectileClass) // Ensure a projectile is defined
    {
        UE_LOG(LogTemp, Warning, TEXT("US_RocketLauncherPrimaryAbility: No ProjectileClass defined in RocketLauncherDataAsset."));
        return false;
    }
    // Super::CanActivateAbility already checks ammo and weapon state.
    return true;
}

void US_RocketLauncherPrimaryAbility::PerformWeaponFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    AS_Character* Character = GetOwningSCharacter();
    AS_RocketLauncher* RocketLauncher = GetRocketLauncher();
    const US_RocketLauncherDataAsset* RocketLauncherData = GetRocketLauncherData();

    if (!Character || !RocketLauncher || !RocketLauncherData || !RocketLauncherData->ProjectileClass || !ActorInfo || !ActorInfo->AbilitySystemComponent.Get())
    {
        CancelAbility(Handle, ActorInfo, ActivationInfo, true);
        return;
    }

    // 1. Apply Ammo Cost (handled by CommitAbility or explicitly here if preferred before fire)
    // Super::ApplyAmmoCost(Handle, ActorInfo, ActivationInfo); // This is called by CommitAbility if costs are defined.

    // 2. Apply Cooldown (handled by CommitAbility or explicitly here)
    // Super::ApplyAbilityCooldown(Handle, ActorInfo, ActivationInfo);

    // 3. Get Aiming Data
    FVector FireStartLocation;
    FVector FireDirection;
    AController* Controller = Character->GetController();
    if (Controller)
    {
        FVector MuzzleSocketLocation = RocketLauncher->GetWeaponMeshComponent()->GetSocketLocation(RocketLauncherData->MuzzleFlashSocketName); // Use MuzzleFlashSocketName from base DA

        // Trace from camera to find target point
        FVector CamLoc, CamRotDir_Unused; // CamRotDir from GetPlayerViewPoint is not normalized aim
        Controller->GetPlayerViewPoint(CamLoc, CamRotDir_Unused);
        FVector CamAimDir = Controller->GetControlRotation().Vector(); // Use controller's control rotation for true aim

        const float MaxFireDistance = 100000.0f; // Large distance for camera trace
        FVector CamTraceEnd = CamLoc + CamAimDir * MaxFireDistance;

        FHitResult CameraTraceHit;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Character);
        QueryParams.AddIgnoredActor(RocketLauncher);

        FireStartLocation = MuzzleSocketLocation; // Projectile starts at muzzle
        if (GetWorld()->LineTraceSingleByChannel(CameraTraceHit, CamLoc, CamTraceEnd, ECC_Visibility, QueryParams))
        {
            FireDirection = (CameraTraceHit.ImpactPoint - MuzzleSocketLocation).GetSafeNormal();
        }
        else {
            FireDirection = (CamTraceEnd - MuzzleSocketLocation).GetSafeNormal(); // Aim towards far point if no hit
        }
    }
    else // AI or no controller? Fallback.
    {
        FireStartLocation = RocketLauncher->GetWeaponMeshComponent()->GetSocketLocation(RocketLauncherData->MuzzleFlashSocketName);
        FireDirection = RocketLauncher->GetActorForwardVector();
    }

    // 4. Play Fire Montage
    UAbilityTask_PlayMontageAndWait* MontageTask = PlayWeaponMontage(
        RocketLauncherData->FireMontage // Assuming FireMontage is used (1P/3P distinction handled in base US_WeaponAbility if properties existed there)
    );
    if (MontageTask)
    {
        // If montage has an event to time the actual projectile spawn:
        // MontageTask->EventReceived.AddDynamic(this, &US_RocketLauncherPrimaryAbility::OnFireEventFromMontage);
        MontageTask->OnCompleted.AddDynamic(this, &US_RocketLauncherPrimaryAbility::OnFireMontageCompleted);
        MontageTask->OnInterrupted.AddDynamic(this, &US_RocketLauncherPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        MontageTask->OnCancelled.AddDynamic(this, &US_RocketLauncherPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        MontageTask->ReadyForActivation();
        // Projectile spawn will happen on event or completion.
    }
    else
    {
        // No montage, fire projectile immediately
        RocketLauncher->ExecuteFire(FireStartLocation, FireDirection, nullptr, 0.f, 0.f, RocketLauncherData->ProjectileClass);
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false); // End immediately if no montage
    }

    // 5. Trigger Muzzle Flash Cue
    if (RocketLauncherData->MuzzleFlashCueTag.IsValid())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = RocketLauncher->GetWeaponMeshComponent()->GetSocketLocation(RocketLauncherData->MuzzleFlashSocketName);
        CueParams.Normal = FireDirection;
        CueParams.Instigator = Character;
        CueParams.EffectCauser = RocketLauncher;
        ActorInfo->AbilitySystemComponent->ExecuteGameplayCue(RocketLauncherData->MuzzleFlashCueTag, CueParams);
    }

    // If no montage task, ability ends after firing. If montage, it ends on montage completion/interruption.
    if (!MontageTask)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }
}