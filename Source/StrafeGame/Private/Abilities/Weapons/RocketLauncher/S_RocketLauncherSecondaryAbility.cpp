#include "Abilities/Weapons/RocketLauncher/S_RocketLauncherSecondaryAbility.h"
#include "Weapons/RocketLauncher/S_RocketLauncher.h"
#include "Weapons/RocketLauncher/S_RocketProjectile.h" // For checking active projectiles
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h" // For ActorInfo

US_RocketLauncherSecondaryAbility::US_RocketLauncherSecondaryAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced; // Simple, one-off action
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated; // Detonation is server-authoritative
    // AbilityInputID is inherited.
    // No cooldown as per description.
    // No ammo cost.
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

    // Check if there are any active rockets to detonate
    bool bHasActiveRockets = false;
    for (const AS_Projectile* Proj : Launcher->GetActiveProjectiles())
    {
        if (Proj && !Proj->IsPendingKill()) // Check if it's a valid, live projectile
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
    if (Launcher && ActorInfo->IsNetAuthority()) // Ensure this runs on server
    {
        Launcher->DetonateOldestActiveRocket();
    }

    // This ability is a single action, so end it immediately.
    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}