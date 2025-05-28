#include "Weapons/RocketLauncher/S_RocketLauncherDataAsset.h"
#include "Weapons/RocketLauncher/S_RocketProjectile.h" // For defaulting ProjectileClass

US_RocketLauncherDataAsset::US_RocketLauncherDataAsset()
{
    // --- Inherited from US_WeaponDataAsset ---
    WeaponDisplayName = FText::FromString(TEXT("Rocket Launcher"));
    // PrimaryFireAbilityClass = US_RocketLauncherPrimaryAbility::StaticClass(); // Assign in Blueprint DA
    // SecondaryFireAbilityClass = US_RocketLauncherSecondaryAbility::StaticClass(); // Assign in Blueprint DA
    // EquipAbilityClass = ...
    // AmmoAttribute = ... (Assign FGameplayAttribute for RocketAmmo)
    // MaxAmmoAttribute = ... (Assign FGameplayAttribute for MaxRocketAmmo)
    // InitialAmmoEffect = ... (Assign GE_GiveInitialRocketAmmo Blueprint)
    // AmmoCostEffect_Primary = ... (Assign GE_Cost_Rocket Primary Blueprint)
    // CooldownTag_Primary = FGameplayTag::RequestGameplayTag(FName("Cooldown.Weapon.RocketLauncher.Primary"));
    // MuzzleFlashCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.RocketLauncher.MuzzleFlash"));

    // --- Inherited from US_ProjectileWeaponDataAsset ---
    // Set the default projectile class for Rocket Launchers
    ProjectileClass = AS_RocketProjectile::StaticClass(); // Default to the C++ base, can be overridden by BP version
    LaunchSpeed = 2200.0f;    // Example default launch speed for rockets
    ProjectileLifeSpan = 6.0f; // Example default lifespan for rockets
    // ProjectileExplosionCueTag is often better defined on the projectile itself or triggered by its detonation logic,
    // but can be here if the weapon always wants a specific cue for its projectiles.

    // --- Specific to US_RocketLauncherDataAsset ---
    // SomeRocketLauncherSpecificStat = 100.f;
}