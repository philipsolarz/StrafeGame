// Source/StrafeGame/Private/Weapons/ChargedShotgun/S_ChargedShotgun.cpp
#include "Weapons/ChargedShotgun/S_ChargedShotgun.h"
#include "Weapons/ChargedShotgun/S_ChargedShotgunDataAsset.h" // Specific DataAsset
#include "Player/S_Character.h"
#include "GameFramework/DamageType.h" // For UDamageType
#include "Net/UnrealNetwork.h"

AS_ChargedShotgun::AS_ChargedShotgun()
{
    // Constructor defaults
    PrimaryChargeProgress = 0.0f;
    SecondaryChargeProgress = 0.0f;
}

void AS_ChargedShotgun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(AS_ChargedShotgun, PrimaryChargeProgress, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(AS_ChargedShotgun, SecondaryChargeProgress, COND_SkipOwner);
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

void AS_ChargedShotgun::SetPrimaryChargeProgress(float Progress)
{
    if (HasAuthority())
    {
        PrimaryChargeProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
        OnRep_PrimaryChargeProgress(); // Call for immediate local update
    }
}

void AS_ChargedShotgun::SetSecondaryChargeProgress(float Progress)
{
    if (HasAuthority())
    {
        SecondaryChargeProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
        OnRep_SecondaryChargeProgress(); // Call for immediate local update
    }
}

void AS_ChargedShotgun::OnRep_PrimaryChargeProgress()
{
    // This is called on clients when the value replicates
    // Can be used to trigger visual/audio effects based on progress
}

void AS_ChargedShotgun::OnRep_SecondaryChargeProgress()
{
    // This is called on clients when the value replicates
    // Can be used to trigger visual/audio effects based on progress
}