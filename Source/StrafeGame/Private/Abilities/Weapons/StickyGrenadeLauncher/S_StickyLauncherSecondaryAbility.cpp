// Source/StrafeGame/Private/Abilities/Weapons/StickyGrenadeLauncher/S_StickyLauncherSecondaryAbility.cpp
#include "Abilities/Weapons/StickyGrenadeLauncher/S_StickyLauncherSecondaryAbility.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncher.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeProjectile.h" 
#include "Weapons/S_Projectile.h" // For AS_Projectile base class
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"

US_StickyGrenadeLauncherSecondaryAbility::US_StickyGrenadeLauncherSecondaryAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
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

    // Check if there's at least one STUCK sticky grenade
    for (const AS_Projectile* Proj : Launcher->GetActiveProjectiles())
    {
        const AS_StickyGrenadeProjectile* StickyProj = Cast<AS_StickyGrenadeProjectile>(Proj);
        if (StickyProj && !StickyProj->IsPendingKillPending() && StickyProj->IsStuckToSurface())
        {
            return true; // Found a stuck sticky, ability can activate
        }
    }

    return false; // No stuck stickies found
}

void US_StickyGrenadeLauncherSecondaryAbility::PerformWeaponSecondaryFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    AS_StickyGrenadeLauncher* Launcher = GetStickyLauncher();
    if (Launcher && ActorInfo->IsNetAuthority())
    {
        // DetonateOldestActiveSticky now only detonates if it's stuck
        Launcher->DetonateOldestActiveSticky();
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}