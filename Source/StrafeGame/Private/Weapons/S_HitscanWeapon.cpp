// Source/StrafeGame/Private/Weapons/S_HitscanWeapon.cpp
#include "Weapons/S_HitscanWeapon.h"
#include "Player/S_Character.h"
#include "Weapons/S_HitscanWeaponDataAsset.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameplayTagsManager.h"
#include "GameFramework/DamageType.h"

AS_HitscanWeapon::AS_HitscanWeapon()
{
    UE_LOG(LogTemp, Log, TEXT("AS_HitscanWeapon::AS_HitscanWeapon: Constructor for %s"), *GetNameSafe(this));
}

void AS_HitscanWeapon::PerformHitscanLogic(
    const FVector& FireStartLocation,
    const FVector& AimDirection,
    const FGameplayEventData& EventData,
    int32 PelletCount,
    float SpreadAngle,
    float MaxRange,
    float BaseDamage,
    TSubclassOf<class UDamageType> DamageTypeClass)
{
    UE_LOG(LogTemp, Log, TEXT("AS_HitscanWeapon::PerformHitscanLogic: %s - Start: %s, Dir: %s, Pellets: %d, Spread: %f, Range: %f, Dmg: %f. HasAuthority: %d"),
        *GetNameSafe(this), *FireStartLocation.ToString(), *AimDirection.ToString(), PelletCount, SpreadAngle, MaxRange, BaseDamage, HasAuthority());

    if (!HasAuthority() || !OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_HitscanWeapon::PerformHitscanLogic: %s - Authority check failed or OwnerCharacter is null."), *GetNameSafe(this));
        return;
    }

    AController* InstigatorController = OwnerCharacter->GetController();
    if (!InstigatorController)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_HitscanWeapon::PerformHitscanLogic: %s - InstigatorController is null."), *GetNameSafe(this));
        return;
    }

    for (int32 i = 0; i < PelletCount; ++i)
    {
        FHitResult HitResult;
        FVector AppliedFireDirection;
        bool bHit = PerformSingleTrace(FireStartLocation, AimDirection, MaxRange, SpreadAngle, HitResult, AppliedFireDirection);

        if (bHit)
        {
            UE_LOG(LogTemp, Verbose, TEXT("AS_HitscanWeapon::PerformHitscanLogic: %s - Pellet %d HIT %s at %s"), *GetNameSafe(this), i, *GetNameSafe(HitResult.GetActor()), *HitResult.ImpactPoint.ToString());
            ProcessHit(HitResult, AppliedFireDirection, OwnerCharacter, InstigatorController, BaseDamage, DamageTypeClass);
        }
        else
        {
            UE_LOG(LogTemp, Verbose, TEXT("AS_HitscanWeapon::PerformHitscanLogic: %s - Pellet %d MISSED. Trace end: %s"), *GetNameSafe(this), i, *(FireStartLocation + AppliedFireDirection * MaxRange).ToString());
        }

#if ENABLE_DRAW_DEBUG
        if (GetWorld()->GetNetMode() != NM_DedicatedServer) // Only draw on clients/listen server
        {
            DrawDebugLine(GetWorld(), FireStartLocation, bHit ? HitResult.ImpactPoint : (FireStartLocation + AppliedFireDirection * MaxRange), FColor::Red, false, 1.0f, 0, 0.5f);
            if (bHit)
            {
                DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 5.f, 8, FColor::Yellow, false, 1.0f);
            }
        }
#endif
    }
    // Muzzle flash and fire sound cues are typically triggered by the GameplayAbility that calls ExecutePrimary/SecondaryFire.
}

bool AS_HitscanWeapon::PerformSingleTrace(const FVector& TraceStart, const FVector& AimDirection, float MaxRange, float SpreadAngleValue, FHitResult& OutHitResult, FVector& OutSpreadAppliedDirection)
{
    OutSpreadAppliedDirection = AimDirection;
    if (SpreadAngleValue > 0.0f)
    {
        const float HalfAngleRad = FMath::DegreesToRadians(SpreadAngleValue * 0.5f);
        OutSpreadAppliedDirection = FMath::VRandCone(AimDirection, HalfAngleRad);
    }

    const FVector TraceEnd = TraceStart + (OutSpreadAppliedDirection * MaxRange);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // Ignore self (the weapon)
    if (OwnerCharacter)
    {
        QueryParams.AddIgnoredActor(OwnerCharacter); // Ignore the owning character
    }
    QueryParams.bReturnPhysicalMaterial = true; // Useful for impact effects

    UE_LOG(LogTemp, VeryVerbose, TEXT("AS_HitscanWeapon::PerformSingleTrace: %s - From: %s To: %s, SpreadAppliedDir: %s"), *GetNameSafe(this), *TraceStart.ToString(), *TraceEnd.ToString(), *OutSpreadAppliedDirection.ToString());

    return GetWorld()->LineTraceSingleByChannel(
        OutHitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility, // Or a custom trace channel for projectiles/weapon fire
        QueryParams
    );
}

void AS_HitscanWeapon::ProcessHit(const FHitResult& HitResult, const FVector& ShotDirection, AS_Character* InstigatorCharacter, AController* InstigatorController, float DamageToApply, TSubclassOf<class UDamageType> DamageTypeClass)
{
    AActor* HitActor = HitResult.GetActor();
    UE_LOG(LogTemp, Log, TEXT("AS_HitscanWeapon::ProcessHit: %s - HitActor: %s, Instigator: %s, Damage: %f"), *GetNameSafe(this), *GetNameSafe(HitActor), *GetNameSafe(InstigatorCharacter), DamageToApply);

    if (HitActor && InstigatorCharacter && InstigatorController)
    {
        UGameplayStatics::ApplyPointDamage(
            HitActor,
            DamageToApply,
            ShotDirection,
            HitResult,
            InstigatorController, // Event Instigator
            this,                 // Damage Causer (the weapon actor)
            DamageTypeClass
        );
        UE_LOG(LogTemp, Log, TEXT("AS_HitscanWeapon %s HIT %s for %f damage. Impact point: %s"), *GetNameSafe(this), *HitActor->GetName(), DamageToApply, *HitResult.ImpactPoint.ToString());

        // GameplayCue for impact effects should be triggered by the ability, passing HitResult if needed
    }
}