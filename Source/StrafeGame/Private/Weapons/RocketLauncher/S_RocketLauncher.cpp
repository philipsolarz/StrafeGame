// Source/StrafeGame/Private/Weapons/RocketLauncher/S_RocketLauncher.cpp
#include "Weapons/RocketLauncher/S_RocketLauncher.h"
#include "Weapons/RocketLauncher/S_RocketProjectile.h"
#include "Weapons/RocketLauncher/S_RocketLauncherDataAsset.h" // Specific DataAsset

AS_RocketLauncher::AS_RocketLauncher()
{
    // Constructor defaults
}

void AS_RocketLauncher::ExecutePrimaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData)
{
    UE_LOG(LogTemp, Log, TEXT("AS_RocketLauncher::ExecutePrimaryFire_Implementation for %s"), *GetNameSafe(this));
    const US_RocketLauncherDataAsset* RocketData = Cast<US_RocketLauncherDataAsset>(GetWeaponData());
    if (!RocketData)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_RocketLauncher::ExecutePrimaryFire: WeaponData is not US_RocketLauncherDataAsset for %s."), *GetNameSafe(this));
        Super::ExecutePrimaryFire_Implementation(FireStartLocation, FireDirection, EventData);
        return;
    }

    if (!RocketData->ProjectileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_RocketLauncher::ExecutePrimaryFire: ProjectileClass is not set in RocketLauncherDataAsset for %s."), *GetNameSafe(this));
        return;
    }

    PerformProjectileSpawnLogic(
        FireStartLocation,
        FireDirection,
        EventData,
        RocketData->ProjectileClass,
        RocketData->LaunchSpeed,
        RocketData->ProjectileLifeSpan
    );
}

void AS_RocketLauncher::ExecuteSecondaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData)
{
    // The Rocket Launcher's secondary ability is to detonate existing rockets, not to fire a different projectile.
    // This function might do nothing, or log that its secondary action is specialized.
    UE_LOG(LogTemp, Log, TEXT("AS_RocketLauncher::ExecuteSecondaryFire_Implementation for %s - Secondary fire for Rocket Launcher is DetonateOldestActiveRocket, called by its ability."), *GetNameSafe(this));
    // Super::ExecuteSecondaryFire_Implementation(FireStartLocation, FireDirection, EventData); // Calls base warning
}

bool AS_RocketLauncher::DetonateOldestActiveRocket()
{
    if (!HasAuthority()) return false;

    if (ActiveProjectiles.Num() > 0)
    {
        AS_Projectile* OldestRocket = nullptr;
        // ActiveProjectiles are added to the end, so the first valid one is the oldest.
        for (AS_Projectile* Proj : ActiveProjectiles)
        {
            if (Proj && !Proj->IsPendingKillPending() && Proj->IsA(AS_RocketProjectile::StaticClass()))
            {
                OldestRocket = Proj;
                break;
            }
        }

        if (OldestRocket)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_RocketLauncher %s: Detonating rocket %s"), *GetName(), *OldestRocket->GetName());
            OldestRocket->Detonate(); // AS_Projectile handles unregistering itself on Detonate/Destroy
            return true;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("AS_RocketLauncher %s: No active rockets to detonate."), *GetName());
    return false;
}