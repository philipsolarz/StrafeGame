// Source/StrafeGame/Private/Weapons/ChargedShotgun/S_ChargedShotgun.cpp
#include "Weapons/ChargedShotgun/S_ChargedShotgun.h"
#include "Weapons/ChargedShotgun/S_ChargedShotgunDataAsset.h" // Specific DataAsset
#include "Player/S_Character.h"
#include "GameFramework/DamageType.h" // For UDamageType

AS_ChargedShotgun::AS_ChargedShotgun()
{
    // Constructor defaults
}

void AS_ChargedShotgun::ExecutePrimaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData)
{
    UE_LOG(LogTemp, Log, TEXT("AS_ChargedShotgun::ExecutePrimaryFire_Implementation for %s"), *GetNameSafe(this));
    const US_ChargedShotgunDataAsset* ChargedData = Cast<US_ChargedShotgunDataAsset>(GetWeaponData());
    if (!ChargedData)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_ChargedShotgun::ExecutePrimaryFire: WeaponData is not US_ChargedShotgunDataAsset for %s."), *GetNameSafe(this));
        Super::ExecutePrimaryFire_Implementation(FireStartLocation, FireDirection, EventData); // Call base to log warning
        return;
    }

    PerformHitscanLogic(
        FireStartLocation,
        FireDirection,
        EventData,
        ChargedData->PrimaryFirePelletCount,
        ChargedData->PrimaryFireSpreadAngle,
        ChargedData->PrimaryFireHitscanRange,
        ChargedData->PrimaryFireDamagePerPellet, // This is damage per pellet
        ChargedData->DamageTypeClass       // DamageTypeClass from base US_HitscanWeaponDataAsset
    );
}

void AS_ChargedShotgun::ExecuteSecondaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData)
{
    UE_LOG(LogTemp, Log, TEXT("AS_ChargedShotgun::ExecuteSecondaryFire_Implementation for %s"), *GetNameSafe(this));
    const US_ChargedShotgunDataAsset* ChargedData = Cast<US_ChargedShotgunDataAsset>(GetWeaponData());
    if (!ChargedData)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_ChargedShotgun::ExecuteSecondaryFire: WeaponData is not US_ChargedShotgunDataAsset for %s."), *GetNameSafe(this));
        Super::ExecuteSecondaryFire_Implementation(FireStartLocation, FireDirection, EventData); // Call base to log warning
        return;
    }

    PerformHitscanLogic(
        FireStartLocation,
        FireDirection,
        EventData,
        ChargedData->SecondaryFirePelletCount,
        ChargedData->SecondaryFireSpreadAngle,
        ChargedData->SecondaryFireHitscanRange,
        ChargedData->SecondaryFireDamagePerPellet, // Damage per pellet for secondary
        ChargedData->DamageTypeClass          // DamageTypeClass from base US_HitscanWeaponDataAsset
    );
}