#include "Weapons/S_HitscanWeaponDataAsset.h"
#include "GameFramework/DamageType.h" // For TSubclassOf<UDamageType>

US_HitscanWeaponDataAsset::US_HitscanWeaponDataAsset()
{
    BaseDamage = 20.0f;
    MaxRange = 10000.0f;
    SpreadAngle = 0.0f;   // Default to accurate
    PelletCount = 1;      // Default to a single trace
    DamageTypeClass = UDamageType::StaticClass(); // Default damage type
}