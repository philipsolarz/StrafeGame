#include "Weapons/S_HitscanWeapon.h"
#include "Player/S_Character.h"
#include "Weapons/S_HitscanWeaponDataAsset.h" // For reading trace parameters, damage, effects, cues
#include "AbilitySystemBlueprintLibrary.h" // For executing GameplayCues
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
//#include "NiagaraFunctionLibrary.h"
//#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h" // For debug traces
#include "GameplayTagsManager.h" // For FGameplayTag
#include "GameFramework/DamageType.h" // For UDamageType

AS_HitscanWeapon::AS_HitscanWeapon()
{
    // BaseDamage = 20.f;
    // DamageTypeClass = UDamageType::StaticClass();
}

void AS_HitscanWeapon::ExecuteFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData, float HitscanSpread, float HitscanRange, TSubclassOf<AS_Projectile> ProjectileClass) // MODIFIED
{
    // Pass EventData along if Super uses it, or use it here.
    // Super::ExecuteFire_Implementation(FireStartLocation, FireDirection, EventData, HitscanSpread, HitscanRange, ProjectileClass); // MODIFIED

    if (!HasAuthority())
    {
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

    const US_HitscanWeaponDataAsset* HitscanData = Cast<US_HitscanWeaponDataAsset>(GetWeaponData());
    if (!HitscanData)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_HitscanWeapon::ExecuteFire: WeaponData is not US_HitscanWeaponDataAsset for %s."), *GetName());
        return;
    }

    int32 FinalPelletCount = HitscanData->PelletCount > 0 ? HitscanData->PelletCount : 1;
    float FinalHitscanRange = HitscanRange > 0 ? HitscanRange : HitscanData->MaxRange; // Use passed-in if valid, else from DA
    float FinalSpreadAngle = HitscanSpread >= 0 ? HitscanSpread : HitscanData->SpreadAngle; // Use passed-in if valid, else from DA


    for (int32 i = 0; i < FinalPelletCount; ++i)
    {
        FHitResult HitResult;
        FVector AppliedFireDirection;
        bool bHit = PerformTrace(FireStartLocation, FireDirection, FinalHitscanRange, FinalSpreadAngle, HitResult, AppliedFireDirection);

        if (bHit)
        {
            ProcessHit(HitResult, AppliedFireDirection, OwnerCharacter, InstigatorController);
            // PlayImpactEffects is now preferably handled by GameplayCue from the Ability
        }
        else
        {
            // PlayImpactEffects for trace miss is also preferably handled by GameplayCue
        }

#if ENABLE_DRAW_DEBUG
        if (GetWorld()->GetNetMode() != NM_DedicatedServer) // Avoid drawing on dedicated server
        {
            DrawDebugLine(GetWorld(), FireStartLocation, bHit ? HitResult.ImpactPoint : (FireStartLocation + AppliedFireDirection * FinalHitscanRange), FColor::Red, false, 1.0f, 0, 0.5f);
            if (bHit) DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 5.f, 8, FColor::Yellow, false, 1.0f);
        }
#endif
    }
    // Muzzle flash cue should be triggered by the GameplayAbility
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