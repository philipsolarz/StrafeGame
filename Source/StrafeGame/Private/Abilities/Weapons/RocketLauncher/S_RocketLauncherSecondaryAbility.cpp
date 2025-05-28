#include "Abilities/Weapons/RocketLauncher/S_RocketLauncherSecondaryAbility.h"
#include "Weapons/RocketLauncher/S_RocketLauncher.h"
#include "Weapons/S_Projectile.h" // For AS_Projectile
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"

US_RocketLauncherSecondaryAbility::US_RocketLauncherSecondaryAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; // CORRECTED from NonInstanced due to warning
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

AS_RocketLauncher* US_RocketLauncherSecondaryAbility::GetRocketLauncher() const
{
    return Cast<AS_RocketLauncher>(GetEquippedWeapon());
}

bool US_RocketLauncherSecondaryAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    AS_RocketLauncher* Launcher = GetRocketLauncher();
    if (!Launcher)
    {
        return false;
    }

    bool bHasActiveRockets = false;
    for (const AS_Projectile* Proj : Launcher->GetActiveProjectiles())
    {
        if (Proj && !Proj->IsPendingKillPending()) // IsPendingKill is AActor method
        {
            bHasActiveRockets = true;
            break;
        }
    }
    return bHasActiveRockets;
}

void US_RocketLauncherSecondaryAbility::PerformWeaponSecondaryFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    AS_RocketLauncher* Launcher = GetRocketLauncher();
    if (Launcher && ActorInfo->IsNetAuthority())
    {
        Launcher->DetonateOldestActiveRocket();
    }
    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}