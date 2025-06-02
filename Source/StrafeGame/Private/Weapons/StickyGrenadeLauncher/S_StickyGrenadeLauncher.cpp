// Source/StrafeGame/Private/Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncher.cpp
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncher.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeProjectile.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncherDataAsset.h" 
#include "Weapons/S_Projectile.h" // For IsPendingKillPending

AS_StickyGrenadeLauncher::AS_StickyGrenadeLauncher()
{
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
}

void AS_StickyGrenadeLauncher::RegisterProjectile(AS_Projectile* Projectile)
{
    Super::RegisterProjectile(Projectile);
    if (HasAuthority()) // Delegate broadcast should be authoritative
    {
        OnActiveProjectilesChanged.Broadcast(GetValidActiveProjectileCount());
    }
}

void AS_StickyGrenadeLauncher::UnregisterProjectile(AS_Projectile* Projectile)
{
    Super::UnregisterProjectile(Projectile);
    if (HasAuthority()) // Delegate broadcast should be authoritative
    {
        OnActiveProjectilesChanged.Broadcast(GetValidActiveProjectileCount());
    }
}

int32 AS_StickyGrenadeLauncher::GetValidActiveProjectileCount() const
{
    int32 ValidCount = 0;
    for (const AS_Projectile* Proj : ActiveProjectiles)
    {
        if (Proj && !Proj->IsPendingKillPending())
        {
            ValidCount++;
        }
    }
    return ValidCount;
}


bool AS_StickyGrenadeLauncher::DetonateOldestActiveSticky()
{
    if (!HasAuthority()) return false;

    if (ActiveProjectiles.Num() > 0)
    {
        AS_StickyGrenadeProjectile* OldestStuckSticky = nullptr;
        for (AS_Projectile* Proj : ActiveProjectiles)
        {
            if (Proj && !Proj->IsPendingKillPending())
            {
                AS_StickyGrenadeProjectile* StickyProj = Cast<AS_StickyGrenadeProjectile>(Proj);
                if (StickyProj && StickyProj->IsStuckToSurface())
                {
                    OldestStuckSticky = StickyProj;
                    break;
                }
            }
        }

        if (OldestStuckSticky)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_StickyGrenadeLauncher %s: Detonating STUCK sticky grenade %s"), *GetName(), *OldestStuckSticky->GetName());
            OldestStuckSticky->Detonate();
            // UnregisterProjectile will be called by AS_Projectile::EndPlay -> AS_ProjectileWeapon::OnProjectileDestroyed
            // which will then trigger our overridden UnregisterProjectile and broadcast the delegate.
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