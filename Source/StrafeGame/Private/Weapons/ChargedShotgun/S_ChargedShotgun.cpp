// Source/StrafeGame/Private/Weapons/ChargedShotgun/S_ChargedShotgun.cpp
#include "Weapons/ChargedShotgun/S_ChargedShotgun.h"
#include "Weapons/ChargedShotgun/S_ChargedShotgunDataAsset.h" 
#include "Player/S_Character.h"
#include "GameFramework/DamageType.h" 
#include "Net/UnrealNetwork.h"

AS_ChargedShotgun::AS_ChargedShotgun()
{
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
        Super::ExecutePrimaryFire_Implementation(FireStartLocation, FireDirection, EventData);
        return;
    }

    PerformHitscanLogic(
        FireStartLocation,
        FireDirection,
        EventData,
        ChargedData->PrimaryFirePelletCount,
        ChargedData->PrimaryFireSpreadAngle,
        ChargedData->PrimaryFireHitscanRange,
        ChargedData->PrimaryFireDamagePerPellet,
        ChargedData->DamageTypeClass
    );
}

void AS_ChargedShotgun::ExecuteSecondaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData)
{
    UE_LOG(LogTemp, Log, TEXT("AS_ChargedShotgun::ExecuteSecondaryFire_Implementation for %s"), *GetNameSafe(this));
    const US_ChargedShotgunDataAsset* ChargedData = Cast<US_ChargedShotgunDataAsset>(GetWeaponData());
    if (!ChargedData)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_ChargedShotgun::ExecuteSecondaryFire: WeaponData is not US_ChargedShotgunDataAsset for %s."), *GetNameSafe(this));
        Super::ExecuteSecondaryFire_Implementation(FireStartLocation, FireDirection, EventData);
        return;
    }

    PerformHitscanLogic(
        FireStartLocation,
        FireDirection,
        EventData,
        ChargedData->SecondaryFirePelletCount,
        ChargedData->SecondaryFireSpreadAngle,
        ChargedData->SecondaryFireHitscanRange,
        ChargedData->SecondaryFireDamagePerPellet,
        ChargedData->DamageTypeClass
    );
}

void AS_ChargedShotgun::SetPrimaryChargeProgress(float Progress)
{
    // This function is typically called on the server by an ability
    if (HasAuthority()) // Or based on your game's prediction model for charge
    {
        float OldProgress = PrimaryChargeProgress;
        PrimaryChargeProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
        // Call OnRep manually on server for local effects and to flag for replication
        OnRep_PrimaryChargeProgress();

        // Broadcast for local ViewModels if progress actually changed
        if (FMath::Abs(PrimaryChargeProgress - OldProgress) > KINDA_SMALL_NUMBER)
        {
            OnPrimaryChargeProgressChanged.Broadcast(PrimaryChargeProgress);
        }
    }
}

void AS_ChargedShotgun::SetSecondaryChargeProgress(float Progress)
{
    // This function is typically called on the server by an ability
    if (HasAuthority()) // Or based on your game's prediction model for charge
    {
        float OldProgress = SecondaryChargeProgress;
        SecondaryChargeProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
        // Call OnRep manually on server for local effects and to flag for replication
        OnRep_SecondaryChargeProgress();

        // Broadcast for local ViewModels if progress actually changed
        if (FMath::Abs(SecondaryChargeProgress - OldProgress) > KINDA_SMALL_NUMBER)
        {
            OnSecondaryChargeProgressChanged.Broadcast(SecondaryChargeProgress);
        }
    }
}

void AS_ChargedShotgun::OnRep_PrimaryChargeProgress()
{
    // This is called on clients when the value replicates
    // And manually on server. Broadcast delegate here too for client-side ViewModels.
    OnPrimaryChargeProgressChanged.Broadcast(PrimaryChargeProgress);
}

void AS_ChargedShotgun::OnRep_SecondaryChargeProgress()
{
    // This is called on clients when the value replicates
    // And manually on server. Broadcast delegate here too for client-side ViewModels.
    OnSecondaryChargeProgressChanged.Broadcast(SecondaryChargeProgress);
}