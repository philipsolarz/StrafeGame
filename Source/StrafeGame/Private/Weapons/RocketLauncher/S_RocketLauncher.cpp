#include "Weapons/RocketLauncher/S_RocketLauncher.h"
#include "Weapons/RocketLauncher/S_RocketProjectile.h" // Specific projectile type
#include "Weapons/S_ProjectileWeaponDataAsset.h" // Or a more specific US_RocketLauncherDataAsset

AS_RocketLauncher::AS_RocketLauncher()
{
    // Constructor defaults for AS_RocketLauncher
}

bool AS_RocketLauncher::DetonateOldestActiveRocket()
{
    if (!HasAuthority()) return false;

    if (ActiveProjectiles.Num() > 0)
    {
        // Sort by spawn time if not already implicitly ordered (TArray Add appends to end)
        // ActiveProjectiles.Sort([](const AS_Projectile& A, const AS_Projectile& B) {
        //     return A.GetCreationTime() < B.GetCreationTime(); // Assuming GetCreationTime() exists or similar
        // });

        AS_Projectile* OldestRocket = nullptr;
        float OldestTime = FLT_MAX;

        for (AS_Projectile* Proj : ActiveProjectiles)
        {
            if (Proj && !Proj->IsPendingKillPending()) // Ensure it's valid and hasn't already been told to detonate
            {
                // Need a way to get spawn time or an incrementing ID if direct time isn't reliable
                // For simplicity, let's assume TArray maintains insertion order, so index 0 is oldest.
                // A more robust way would be to store spawn time on projectile.
                // For now, let's just take the first valid one.
                OldestRocket = Proj; // This will be the first one added if TArray is used like a queue
                break;
            }
        }

        if (OldestRocket)
        {
            // Ensure it's actually a rocket if we have multiple projectile types from this weapon (unlikely for RL)
            if (Cast<AS_RocketProjectile>(OldestRocket))
            {
                UE_LOG(LogTemp, Log, TEXT("AS_RocketLauncher %s: Detonating rocket %s"), *GetName(), *OldestRocket->GetName());
                OldestRocket->Detonate(); // AS_Projectile handles unregistering itself on Detonate/Destroy
                return true;
            }
        }
    }
    return false;
}