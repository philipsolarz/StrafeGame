#include "Weapons/ChargedShotgun/S_ChargedShotgunDataAsset.h"
#include "GameplayEffect.h" // For TSubclassOf<UGameplayEffect>
#include "Sound/SoundBase.h"   // For TSoftObjectPtr<USoundBase>

US_ChargedShotgunDataAsset::US_ChargedShotgunDataAsset()
{
    // --- Inherited from US_HitscanWeaponDataAsset (can be overridden here or in BP DA instance) ---
    // BaseDamage = 10.0f; // Example if primary fire pellets do 10 damage each
    // MaxRange = 5000.0f;
    // SpreadAngle = 5.0f;
    // PelletCount = 8; // Example for primary fire

    // --- Primary Fire: Charge & Auto-Fire ---
    PrimaryChargeTime = 1.0f;
    PrimaryEarlyReleaseCooldownEffect = nullptr; // Assign the GE Blueprint subclass here
    PrimaryFirePelletCount = 8;
    PrimaryFireSpreadAngle = 7.0f;
    PrimaryFireHitscanRange = 4000.0f;
    PrimaryFireDamagePerPellet = 10.0f;

    // --- Secondary Fire: Charge & Hold, Fire on Release ---
    SecondaryChargeTime = 2.0f;
    SecondaryFireLockoutEffect = nullptr; // Assign the GE Blueprint subclass here
    // SecondaryFireLockoutTag should be the tag *applied by* SecondaryFireLockoutEffect
    SecondaryFireLockoutTag = FGameplayTag::RequestGameplayTag(FName("State.Weapon.ChargedShotgun.Lockout")); // Example Tag
    SecondaryFirePelletCount = 12;
    SecondaryFireSpreadAngle = 3.0f;
    SecondaryFireHitscanRange = 6000.0f;
    SecondaryFireDamagePerPellet = 15.0f;


    // --- Gameplay Cues for Charging ---
    PrimaryChargeStartCue = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.ChargedShotgun.ChargeStart.Primary"));
    PrimaryChargeLoopCue = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.ChargedShotgun.ChargeLoop.Primary"));
    PrimaryChargeCompleteCue = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.ChargedShotgun.ChargeComplete.Primary"));
    SecondaryChargeStartCue = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.ChargedShotgun.ChargeStart.Secondary"));
    SecondaryChargeLoopCue = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.ChargedShotgun.ChargeLoop.Secondary"));
    SecondaryChargeHeldCue = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.ChargedShotgun.ChargeHeld.Secondary"));
    SecondaryOverchargedFireCue = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.ChargedShotgun.Fire.Overcharged"));

    // --- Sounds ---
    PrimaryChargeSound = nullptr;
    PrimaryFireChargedSound = nullptr;
    SecondaryChargeSound = nullptr;
    SecondaryChargeHeldSound = nullptr;
    SecondaryFireOverchargedSound = nullptr;

    // Ensure base class (US_WeaponDataAsset) properties are also set if needed
    // e.g., WeaponDisplayName, specific primary/secondary ability classes, ammo attributes
    // AmmoAttribute = YourShotgunAmmoAttribute;
    // MaxAmmoAttribute = YourMaxShotgunAmmoAttribute;
    // InitialAmmoEffect = YourShotgunInitialAmmoGE;
    // AmmoCostEffect_Primary = YourShotgunPrimaryCostGE;
    // AmmoCostEffect_Secondary = YourShotgunSecondaryCostGE;
    // CooldownTag_Primary = FGameplayTag::RequestGameplayTag(FName("Cooldown.Weapon.Shotgun.Primary"));
    // MuzzleFlashCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.Shotgun.MuzzleFlash"));
}