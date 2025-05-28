#include "Abilities/Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncherSecondaryAbility.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncher.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeProjectile.h" // For checking active projectiles
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"

US_StickyGrenadeLauncherSecondaryAbility::US_StickyGrenadeLauncherSecondaryAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated; // Detonation is server-authoritative
    // No cooldown, no ammo cost.
}

AS_StickyGrenadeLauncher* US_StickyGrenadeLauncherSecondaryAbility::GetStickyLauncher() const
{
    return Cast<AS_StickyGrenadeLauncher>(GetEquippedWeapon());
}

bool US_StickyGrenadeLauncherSecondaryAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    AS_StickyGrenadeLauncher* Launcher = GetStickyLauncher();
    if (!Launcher)
    {
        return false;
    }

    // Check if there are any active stickies to detonate
    bool bHasActiveStickies = false;
    for (const AS_Projectile* Proj : Launcher->GetActiveProjectiles())
    {
        if (Proj && Cast<AS_StickyGrenadeProjectile>(Proj) && !Proj->IsPendingKill())
        {
            bHasActiveStickies = true;
            break;
        }
    }
    return bHasActiveStickies;
}

void US_StickyGrenadeLauncherSecondaryAbility::PerformWeaponSecondaryFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    AS_StickyGrenadeLauncher* Launcher = GetStickyLauncher();
    if (Launcher && ActorInfo->IsNetAuthority()) // Server-authoritative action
    {
        Launcher->DetonateOldestActiveSticky();
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, false, false); // Single action, end immediately
}