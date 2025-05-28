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

void AS_HitscanWeapon::ExecuteFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData, float HitscanSpread, float HitscanRange, TSubclassOf<AS_Projectile> ProjectileClass)
{
    UE_LOG(LogTemp, Log, TEXT("AS_HitscanWeapon::ExecuteFire_Implementation: %s - FireStart: %s, FireDir: %s, Spread: %f, Range: %f. HasAuthority: %d"),
         *GetNameSafe(this), *FireStartLocation.ToString(), *FireDirection.ToString(), HitscanSpread, HitscanRange, HasAuthority());

    if (!HasAuthority())
    {
        return;
    }

    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_HitscanWeapon::ExecuteFire_Implementation: %s - OwnerCharacter is null."), *GetNameSafe(this));
        return;
    }

    AController* InstigatorController = OwnerCharacter->GetController();
    if (!InstigatorController)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_HitscanWeapon::ExecuteFire_Implementation: %s - InstigatorController is null."), *GetNameSafe(this));
        return;
    }

    const US_HitscanWeaponDataAsset* HitscanData = Cast<US_HitscanWeaponDataAsset>(GetWeaponData());
    if (!HitscanData)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_HitscanWeapon::ExecuteFire_Implementation: %s - WeaponData is not US_HitscanWeaponDataAsset."), *GetNameSafe(this));
        return;
    }

    int32 FinalPelletCount = HitscanData->PelletCount > 0 ? HitscanData->PelletCount : 1;
    float FinalHitscanRange = HitscanRange > 0 ? HitscanRange : HitscanData->MaxRange;
    float FinalSpreadAngle = HitscanSpread >= 0 ? HitscanSpread : HitscanData->SpreadAngle;
    UE_LOG(LogTemp, Verbose, TEXT("AS_HitscanWeapon::ExecuteFire_Implementation: %s - FinalPellets: %d, FinalRange: %f, FinalSpread: %f"), 
         *GetNameSafe(this), FinalPelletCount, FinalHitscanRange, FinalSpreadAngle);


    for (int32 i = 0; i < FinalPelletCount; ++i)
    {
        FHitResult HitResult;
        FVector AppliedFireDirection;
        bool bHit = PerformTrace(FireStartLocation, FireDirection, FinalHitscanRange, FinalSpreadAngle, HitResult, AppliedFireDirection);

        if (bHit)
        {
            UE_LOG(LogTemp, Verbose, TEXT("AS_HitscanWeapon::ExecuteFire_Implementation: %s - Pellet %d HIT %s at %s"), *GetNameSafe(this), i, *GetNameSafe(HitResult.GetActor()), *HitResult.ImpactPoint.ToString());
            ProcessHit(HitResult, AppliedFireDirection, OwnerCharacter, InstigatorController);
        }
        else
        {
            UE_LOG(LogTemp, Verbose, TEXT("AS_HitscanWeapon::ExecuteFire_Implementation: %s - Pellet %d MISSED. Trace end: %s"), *GetNameSafe(this), i, *(FireStartLocation + AppliedFireDirection * FinalHitscanRange).ToString());
        }

#if ENABLE_DRAW_DEBUG
        if (GetWorld()->GetNetMode() != NM_DedicatedServer)
        {
            DrawDebugLine(GetWorld(), FireStartLocation, bHit ? HitResult.ImpactPoint : (FireStartLocation + AppliedFireDirection * FinalHitscanRange), FColor::Red, false, 1.0f, 0, 0.5f);
            if (bHit) DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 5.f, 8, FColor::Yellow, false, 1.0f);
        }
#endif
    }
}

bool AS_HitscanWeapon::PerformTrace(const FVector& TraceStart, const FVector& AimDirection, float MaxRange, float SpreadAngle, FHitResult& OutHitResult, FVector& SpreadAppliedDirection)
{
    SpreadAppliedDirection = AimDirection;
    if (SpreadAngle > 0.0f)
    {
        const float HalfAngleRad = FMath::DegreesToRadians(SpreadAngle * 0.5f);
        SpreadAppliedDirection = FMath::VRandCone(AimDirection, HalfAngleRad);
    }

    const FVector TraceEnd = TraceStart + (SpreadAppliedDirection * MaxRange);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    if (OwnerCharacter) QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bReturnPhysicalMaterial = true;

    UE_LOG(LogTemp, VeryVerbose, TEXT("AS_HitscanWeapon::PerformTrace: %s - From: %s To: %s, SpreadAppliedDir: %s"), *GetNameSafe(this), *TraceStart.ToString(), *TraceEnd.ToString(), *SpreadAppliedDirection.ToString());

    return GetWorld()->LineTraceSingleByChannel(
        OutHitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );
}

void AS_HitscanWeapon::ProcessHit(const FHitResult& HitResult, const FVector& ShotDirection, AS_Character* InstigatorCharacter, AController* InstigatorController)
{
    AActor* HitActor = HitResult.GetActor();
    UE_LOG(LogTemp, Log, TEXT("AS_HitscanWeapon::ProcessHit: %s - HitActor: %s, Instigator: %s"), *GetNameSafe(this), *GetNameSafe(HitActor), *GetNameSafe(InstigatorCharacter));
    if (HitActor && InstigatorCharacter && InstigatorController)
    {
        float BaseDamage = 20.0f;
        TSubclassOf<UDamageType> DamageType = UDamageType::StaticClass();

        if (const US_HitscanWeaponDataAsset* HitscanData = Cast<US_HitscanWeaponDataAsset>(WeaponData))
        {
            BaseDamage = HitscanData->BaseDamage;
            DamageType = HitscanData->DamageTypeClass;
            UE_LOG(LogTemp, Verbose, TEXT("AS_HitscanWeapon::ProcessHit: %s - Using Damage: %f, Type: %s from HitscanDataAsset"), *GetNameSafe(this), BaseDamage, *GetNameSafe(DamageType));
        }
        else if (WeaponData) // Fallback to generic WeaponData if HitscanDataAsset cast fails but base WeaponData exists
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_HitscanWeapon::ProcessHit: %s - WeaponData is not US_HitscanWeaponDataAsset. Falling back to potential base damage/type if they existed there."), *GetNameSafe(this));
           // (No BaseDamage or DamageTypeClass in US_WeaponDataAsset, so this would use placeholders)
        }


        UGameplayStatics::ApplyPointDamage(
            HitActor,
            BaseDamage,
            ShotDirection,
            HitResult,
            InstigatorController,
            this,
            DamageType
        );
        UE_LOG(LogTemp, Log, TEXT("AS_HitscanWeapon %s HIT %s for %f damage. Impact point: %s"), *GetNameSafe(this), *HitActor->GetName(), BaseDamage, *HitResult.ImpactPoint.ToString());
    }
}

void AS_HitscanWeapon::PlayImpactEffects(const FVector& EndPoint, const FHitResult* HitResultOptional, bool bHitTarget)
{
    UE_LOG(LogTemp, Verbose, TEXT("AS_HitscanWeapon::PlayImpactEffects: %s - EndPoint: %s, bHitTarget: %d. (Effects are now preferably handled by GameplayCues triggered by Abilities)"), 
         *GetNameSafe(this), *EndPoint.ToString(), bHitTarget);
}