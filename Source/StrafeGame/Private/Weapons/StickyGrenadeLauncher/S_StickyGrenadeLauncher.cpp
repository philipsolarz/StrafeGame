#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncher.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeProjectile.h"
#include "Weapons/S_ProjectileWeaponDataAsset.h" // Or US_StickyLauncherDataAsset

AS_StickyGrenadeLauncher::AS_StickyGrenadeLauncher()
{
    // MaxActiveStickies = 3; // Default, but this check should be in the fire ability
}

bool AS_StickyGrenadeLauncher::DetonateOldestActiveSticky()
{
    if (!HasAuthority()) return false;

    if (ActiveProjectiles.Num() > 0)
    {
        AS_StickyGrenadeProjectile* OldestSticky = nullptr;

        // Iterate to find the first valid AS_StickyGrenadeProjectile.
        // ActiveProjectiles are added to the end, so index 0 is oldest.
        for (AS_Projectile* Proj : ActiveProjectiles)
        {
            if (Proj && !Proj->IsPendingKill())
            {
                OldestSticky = Cast<AS_StickyGrenadeProjectile>(Proj);
                if (OldestSticky) break; // Found the first valid sticky
            }
        }

        if (OldestSticky)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_StickyGrenadeLauncher %s: Detonating sticky grenade %s"), *GetName(), *OldestSticky->GetName());
            OldestSticky->Detonate(); // AS_Projectile handles unregistering itself
            return true;
        }
    }
    return false;
}