#include "Weapons/S_ProjectileWeaponDataAsset.h"
#include "Weapons/S_Projectile.h" // For TSubclassOf<AS_Projectile>

US_ProjectileWeaponDataAsset::US_ProjectileWeaponDataAsset()
{
    ProjectileClass = nullptr;
    LaunchSpeed = 3000.0f; // Example default
    ProjectileLifeSpan = 5.0f;  // Example default
}