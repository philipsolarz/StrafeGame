#include "Weapons/S_ProjectileWeapon.h"
#include "Weapons/S_Projectile.h"         // For AS_Projectile
#include "Player/S_Character.h"
#include "Weapons/S_ProjectileWeaponDataAsset.h"    // For getting projectile class
#include "Engine/World.h"                 // For spawning
#include "GameFramework/Controller.h"     // For AController
#include "GameFramework/ProjectileMovementComponent.h"

AS_ProjectileWeapon::AS_ProjectileWeapon()
{
    // DefaultProjectileClass = nullptr; // To be set in derived BPs or read from DataAsset by GA
}

void AS_ProjectileWeapon::ExecuteFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData, float HitscanSpread, float HitscanRange, TSubclassOf<AS_Projectile> ProjectileClassFromAbility)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_ProjectileWeapon::ExecuteFire: OwnerCharacter is null for %s."), *GetName());
        return;
    }

    AController* InstigatorController = OwnerCharacter->GetController();
    if (!InstigatorController)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_ProjectileWeapon::ExecuteFire: InstigatorController is null for %s."), *GetName());
        return;
    }

    const US_ProjectileWeaponDataAsset* ProjWeaponDataAssetFromWeapon = Cast<US_ProjectileWeaponDataAsset>(GetWeaponData()); // Correctly cast here too
    TSubclassOf<AS_Projectile> ActualProjectileClass = ProjectileClassFromAbility;

    if (!ActualProjectileClass && ProjWeaponDataAssetFromWeapon)
    {
        ActualProjectileClass = ProjWeaponDataAssetFromWeapon->ProjectileClass;
    }

    if (!ActualProjectileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_ProjectileWeapon::ExecuteFire: No ProjectileClass specified for %s."), *GetName());
        return;
    }

    FTransform SpawnTransform(FireDirection.Rotation(), FireStartLocation);
    AS_Projectile* SpawnedProjectile = SpawnProjectile(ActualProjectileClass, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), OwnerCharacter, InstigatorController);

}

AS_Projectile* AS_ProjectileWeapon::SpawnProjectile(TSubclassOf<AS_Projectile> ProjectileClass, const FVector& SpawnLocation, const FRotator& SpawnRotation, AS_Character* InstigatorCharacter, AController* InstigatorController)
{
    UWorld* const World = GetWorld();
    if (!World || !ProjectileClass || !InstigatorCharacter || !InstigatorController)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = InstigatorCharacter;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;


    AS_Projectile* Projectile = World->SpawnActor<AS_Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
    if (Projectile)
    {
        // Get the correct data asset type
        const US_ProjectileWeaponDataAsset* ProjWeaponData = Cast<US_ProjectileWeaponDataAsset>(GetWeaponData());
        Projectile->InitializeProjectile(OwnerCharacter, this, ProjWeaponData); // Pass the casted data asset

        if (ProjWeaponData) // Check if cast was successful
        {
            if (ProjWeaponData->LaunchSpeed > 0.f && Projectile->ProjectileMovementComponent) {
                Projectile->ProjectileMovementComponent->InitialSpeed = ProjWeaponData->LaunchSpeed;
                Projectile->ProjectileMovementComponent->MaxSpeed = ProjWeaponData->LaunchSpeed;
            }
            if (ProjWeaponData->ProjectileLifeSpan > 0.f) {
                Projectile->SetLifeSpan(ProjWeaponData->ProjectileLifeSpan);
            }
        }
        // Else: if ProjWeaponData is null, InitializeProjectile will receive nullptr for the data asset,
        // which it should handle gracefully (e.g., by using projectile's internal defaults).
        // Or, you could log a warning if the cast fails and the data asset is expected.

        RegisterProjectile(Projectile);
    }
    return Projectile;
}

void AS_ProjectileWeapon::RegisterProjectile(AS_Projectile* Projectile)
{
    if (HasAuthority() && Projectile && !ActiveProjectiles.Contains(Projectile))
    {
        ActiveProjectiles.Add(Projectile);
        Projectile->OnDestroyed.AddUniqueDynamic(this, &AS_ProjectileWeapon::OnProjectileDestroyed);
    }
}

void AS_ProjectileWeapon::UnregisterProjectile(AS_Projectile* Projectile)
{
    if (HasAuthority() && Projectile)
    {
        ActiveProjectiles.Remove(Projectile);
    }
}

void AS_ProjectileWeapon::OnProjectileDestroyed(AActor* DestroyedActor)
{
    if (HasAuthority())
    {
        AS_Projectile* Projectile = Cast<AS_Projectile>(DestroyedActor);
        if (Projectile)
        {
            ActiveProjectiles.Remove(Projectile);
        }
    }
}