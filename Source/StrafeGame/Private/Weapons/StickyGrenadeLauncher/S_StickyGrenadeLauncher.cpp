// Source/StrafeGame/Private/Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncher.cpp
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncher.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeProjectile.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncherDataAsset.h" // Specific DataAsset

AS_StickyGrenadeLauncher::AS_StickyGrenadeLauncher()
{
    // Constructor
}

void AS_StickyGrenadeLauncher::ExecutePrimaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData)
{
    UE_LOG(LogTemp, Log, TEXT("AS_StickyGrenadeLauncher::ExecutePrimaryFire_Implementation for %s"), *GetNameSafe(this));
    const US_StickyGrenadeLauncherDataAsset* StickyData = Cast<US_StickyGrenadeLauncherDataAsset>(GetWeaponData());
    if (!StickyData)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StickyGrenadeLauncher::ExecutePrimaryFire: WeaponData is not US_StickyGrenadeLauncherDataAsset for %s."), *GetNameSafe(this));
        Super::ExecutePrimaryFire_Implementation(FireStartLocation, FireDirection, EventData);
        return;
    }
    if (!StickyData->ProjectileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StickyGrenadeLauncher::ExecutePrimaryFire: ProjectileClass is not set in StickyLauncherDataAsset for %s."), *GetNameSafe(this));
        return;
    }
    // Check for max active stickies is handled by the ability (US_StickyLauncherPrimaryAbility::CanActivateAbility) before this is called.
    PerformProjectileSpawnLogic(
        FireStartLocation,
        FireDirection,
        EventData,
        StickyData->ProjectileClass,
        StickyData->LaunchSpeed,
        StickyData->ProjectileLifeSpan
    );
}

void AS_StickyGrenadeLauncher::ExecuteSecondaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData)
{
    UE_LOG(LogTemp, Log, TEXT("AS_StickyGrenadeLauncher::ExecuteSecondaryFire_Implementation for %s - Secondary fire for Sticky Launcher is DetonateOldestActiveSticky, called by its ability."), *GetNameSafe(this));
    // Super::ExecuteSecondaryFire_Implementation(FireStartLocation, FireDirection, EventData); // Calls base warning
}

bool AS_StickyGrenadeLauncher::DetonateOldestActiveSticky()
{
    if (!HasAuthority()) return false;

    if (ActiveProjectiles.Num() > 0)
    {
        AS_StickyGrenadeProjectile* OldestStuckSticky = nullptr;
        // Iterate to find the first (oldest) active projectile that is a STUCK sticky grenade
        for (AS_Projectile* Proj : ActiveProjectiles)
        {
            if (Proj && !Proj->IsPendingKillPending()) // Ensure it's valid
            {
                AS_StickyGrenadeProjectile* StickyProj = Cast<AS_StickyGrenadeProjectile>(Proj);
                if (StickyProj && StickyProj->IsStuckToSurface()) // MODIFIED: Check if it's stuck
                {
                    OldestStuckSticky = StickyProj;
                    break;
                }
            }
        }

        if (OldestStuckSticky)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_StickyGrenadeLauncher %s: Detonating STUCK sticky grenade %s"), *GetName(), *OldestStuckSticky->GetName());
            OldestStuckSticky->Detonate(); // AS_Projectile handles unregistering itself
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("AS_StickyGrenadeLauncher %s: No active STUCK stickies to detonate."), *GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AS_StickyGrenadeLauncher %s: No active projectiles to detonate."), *GetName());
    }
    return false;
}