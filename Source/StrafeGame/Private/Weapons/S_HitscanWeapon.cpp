#include "Weapons/S_HitscanWeapon.h"
#include "Player/S_Character.h"
#include "Weapons/S_WeaponDataAsset.h" // For reading trace parameters, damage, effects, cues
#include "AbilitySystemBlueprintLibrary.h" // For executing GameplayCues
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h" // For debug traces
#include "GameplayTagsManager.h" // For FGameplayTag
#include "GameFramework/DamageType.h" // For UDamageType

AS_HitscanWeapon::AS_HitscanWeapon()
{
    // BaseDamage = 20.f;
    // DamageTypeClass = UDamageType::StaticClass();
}

void AS_HitscanWeapon::ExecuteFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData* EventData, float HitscanSpread, float HitscanRange, TSubclassOf<AS_Projectile> ProjectileClass)
{
    Super::ExecuteFire_Implementation(FireStartLocation, FireDirection, EventData, HitscanSpread, HitscanRange, ProjectileClass);

    if (GetOwnerRole() != ROLE_Authority) // Server performs traces and applies damage
    {
        // Client might play cosmetic muzzle flash here if not handled by ability's gameplay cue
        return;
    }

    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_HitscanWeapon::ExecuteFire: OwnerCharacter is null for %s."), *GetName());
        return;
    }

    AController* InstigatorController = OwnerCharacter->GetController();
    if (!InstigatorController)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_HitscanWeapon::ExecuteFire: InstigatorController is null for %s."), *GetName());
        return;
    }

    const US_WeaponDataAsset* CurrentWeaponData = GetWeaponData();
    if (!CurrentWeaponData)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_HitscanWeapon::ExecuteFire: WeaponData is null for %s."), *GetName());
        return;
    }

    // Parameters from WeaponDataAsset (or passed in by ability if ability modifies them)
    // For a shotgun, PelletCount would be > 1. This loop would run PelletCount times.
    // The HitscanSpread and HitscanRange are passed in, potentially from the WeaponDataAsset via the Ability.
    // BaseDamage also comes from WeaponDataAsset, usually applied within the GameplayAbility or here if GA delegates damage.

    int32 PelletCount = 1; // Default to 1 for a rifle-like weapon. Shotguns override this.
    // This should ideally be configured in the WeaponDataAsset or passed by the GA.
    float DamagePerPellet = 20.f; // From WeaponDataAsset or GA
    TSubclassOf<UDamageType> DamageType = UDamageType::StaticClass(); // From WeaponDataAsset or GA

    // Example: Reading pellet count from WeaponDataAsset if it exists there for this weapon type
    // if (CurrentWeaponData->WeaponStats.IsA(FHitscanWeaponStats::StaticStruct()) ) // pseudo-code for checking stats type
    // {
    //     const FHitscanWeaponStats* HitscanStats = CurrentWeaponData->GetStats<FHitscanWeaponStats>();
    //     PelletCount = HitscanStats->PelletCount;
    //     DamagePerPellet = HitscanStats->DamagePerPellet;
    // }


    for (int32 i = 0; i < PelletCount; ++i)
    {
        FHitResult HitResult;
        FVector AppliedFireDirection; // The actual direction after spread
        bool bHit = PerformTrace(FireStartLocation, FireDirection, HitscanRange, HitscanSpread, HitResult, AppliedFireDirection);

        if (bHit)
        {
            ProcessHit(HitResult, AppliedFireDirection, OwnerCharacter, InstigatorController);
            PlayImpactEffects(HitResult.ImpactPoint, &HitResult, true);
        }
        else
        {
            PlayImpactEffects(FireStartLocation + AppliedFireDirection * HitscanRange, nullptr, false);
        }

        // Debug drawing
#if ENABLE_DRAW_DEBUG
        if (GetWorld()->GetNetMode() != NM_DedicatedServer)
        {
            DrawDebugLine(GetWorld(), FireStartLocation, bHit ? HitResult.ImpactPoint : (FireStartLocation + AppliedFireDirection * HitscanRange), FColor::Red, false, 1.0f, 0, 0.5f);
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
    QueryParams.AddIgnoredActor(this); // Ignore the weapon itself
    if (OwnerCharacter) QueryParams.AddIgnoredActor(OwnerCharacter); // Ignore the firer
    QueryParams.bReturnPhysicalMaterial = true;

    return GetWorld()->LineTraceSingleByChannel(
        OutHitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility, // Consider a custom "WeaponTrace" channel
        QueryParams
    );
}

void AS_HitscanWeapon::ProcessHit(const FHitResult& HitResult, const FVector& ShotDirection, AS_Character* InstigatorCharacter, AController* InstigatorController)
{
    // This function is server-authoritative.
    AActor* HitActor = HitResult.GetActor();
    if (HitActor && InstigatorCharacter && InstigatorController)
    {
        // Damage application logic should ideally be part of a GameplayEffect executed by the firing GameplayAbility.
        // The ability would gather hit info (from an GAT_WaitTargetData style task) and then apply a GE.
        // However, if the weapon actor itself is responsible for point damage:
        float BaseDamage = 20.0f; // Placeholder: Get from WeaponData or Ability
        TSubclassOf<UDamageType> DamageType = UDamageType::StaticClass(); // Placeholder

        if (WeaponData)
        {
            // BaseDamage = WeaponData->WeaponStats.BaseDamage; // Assuming BaseDamage exists in your FWeaponStats
            // DamageType = WeaponData->DamageTypeClass;      // Assuming DamageTypeClass exists
        }

        UGameplayStatics::ApplyPointDamage(
            HitActor,
            BaseDamage,
            ShotDirection,
            HitResult,
            InstigatorController, // Event Instigator
            this,                 // Damage Causer (the weapon)
            DamageType
        );
        UE_LOG(LogTemp, Log, TEXT("AS_HitscanWeapon %s hit %s for %f damage."), *GetName(), *HitActor->GetName(), BaseDamage);
    }
}

void AS_HitscanWeapon::PlayImpactEffects(const FVector& EndPoint, const FHitResult* HitResultOptional, bool bHitTarget)
{
    // This function can be called on server and clients.
    // Effects should ideally be triggered by a GameplayCue.
    // The GameplayAbility that calls ExecuteFire should get target data and then execute a GameplayCue.

    if (WeaponData && WeaponData->ImpactEffectCueTag.IsValid() && OwnerCharacter) // ImpactCueTag from US_WeaponDataAsset
    {
        UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerCharacter); // Get ASC from character (or its PlayerState)
        if (ASC)
        {
            FGameplayCueParameters CueParams;
            CueParams.Location = EndPoint;
            if (HitResultOptional)
            {
                CueParams.Normal = HitResultOptional->ImpactNormal;
                CueParams.PhysicalMaterial = HitResultOptional->PhysMaterial;
                CueParams.TargetAttachComponent = HitResultOptional->GetComponent();
            }
            CueParams.Instigator = OwnerCharacter;
            // CueParams.EffectContext = ASC->MakeEffectContext(); // Context for the cue
            // if (CueParams.EffectContext.IsValid()) CueParams.EffectContext.AddSourceObject(this);

            FGameplayTag FinalImpactCueTag = WeaponData->ImpactEffectCueTag;
            // You could have more specific tags based on HitResultOptional->PhysMaterial
            // e.g., FinalImpactCueTag = MyGameplayTags::Get()->GameplayCue_Impact_Weapon_Flesh;

            ASC->ExecuteGameplayCue(FinalImpactCueTag, CueParams);
        }
    }
    else
    {
        // Fallback to direct effect spawning if no cue specified or no ASC
        // UE_LOG(LogTemp, Warning, TEXT("AS_HitscanWeapon %s: No ImpactEffectCueTag or Owner ASC for impact effects."), *GetName());
        // if (WeaponData && WeaponData->DefaultImpactEffect) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WeaponData->DefaultImpactEffect, EndPoint);
    }
}