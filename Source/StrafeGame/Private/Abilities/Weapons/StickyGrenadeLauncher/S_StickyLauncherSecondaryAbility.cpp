#include "Abilities/Weapons/StickyGrenadeLauncher/S_StickyLauncherSecondaryAbility.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncher.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeProjectile.h" 
#include "Weapons/S_Projectile.h" // For AS_Projectile base class
#include "Player/S_Character.h"
#include "AbilitySystemComponent.h"

US_StickyGrenadeLauncherSecondaryAbility::US_StickyGrenadeLauncherSecondaryAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; // CORRECTED from NonInstanced due to warning
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

    bool bHasActiveStickies = false;
    for (const AS_Projectile* Proj : Launcher->GetActiveProjectiles())
    {
        // Check if it's a sticky grenade and not pending kill
        if (Proj && Cast<AS_StickyGrenadeProjectile>(Proj) && !Proj->IsPendingKill()) // IsPendingKill is AActor method
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
    if (Launcher && ActorInfo->IsNetAuthority())
    {
        Launcher->DetonateOldestActiveSticky();
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}