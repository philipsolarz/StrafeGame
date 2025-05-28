#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncherDataAsset.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeProjectile.h" // For defaulting ProjectileClass

US_StickyGrenadeLauncherDataAsset::US_StickyGrenadeLauncherDataAsset()
{
    // --- Inherited from US_WeaponDataAsset ---
    WeaponDisplayName = FText::FromString(TEXT("Sticky Grenade Launcher"));
    // PrimaryFireAbilityClass = US_StickyGrenadeLauncherPrimaryAbility::StaticClass(); // Assign in Blueprint DA
    // SecondaryFireAbilityClass = US_StickyGrenadeLauncherSecondaryAbility::StaticClass(); // Assign in Blueprint DA
    // AmmoAttribute = ... (Assign FGameplayAttribute for StickyGrenadeAmmo)
    // MaxAmmoAttribute = ... (Assign FGameplayAttribute for MaxStickyGrenadeAmmo)
    // InitialAmmoEffect = ... (Assign GE_GiveInitialStickyGrenades Blueprint)
    // AmmoCostEffect_Primary = ... (Assign GE_Cost_StickyGrenade Blueprint)
    // CooldownTag_Primary = FGameplayTag::RequestGameplayTag(FName("Cooldown.Weapon.StickyLauncher.Primary")); // For 0.333s fire rate
    // MuzzleFlashCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.StickyLauncher.MuzzleFlash"));

    // --- Inherited from US_ProjectileWeaponDataAsset ---
    ProjectileClass = AS_StickyGrenadeProjectile::StaticClass(); // Default to C++ base
    LaunchSpeed = 1800.0f;    // Stickies are often a bit slower
    ProjectileLifeSpan = 60.0f; // Stickies can last a long time if not detonated

    // --- Specific to US_StickyGrenadeLauncherDataAsset ---
    MaxActiveProjectiles = 3; // As per your requirement
}