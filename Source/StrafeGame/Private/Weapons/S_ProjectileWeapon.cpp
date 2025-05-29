// Source/StrafeGame/Private/Weapons/S_ProjectileWeapon.cpp
#include "Weapons/S_ProjectileWeapon.h"
#include "Weapons/S_Projectile.h"
#include "Player/S_Character.h"
#include "Weapons/S_ProjectileWeaponDataAsset.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/ProjectileMovementComponent.h" // For setting initial speed

AS_ProjectileWeapon::AS_ProjectileWeapon()
{
    UE_LOG(LogTemp, Log, TEXT("AS_ProjectileWeapon::AS_ProjectileWeapon: Constructor for %s"), *GetNameSafe(this));
}

AS_Projectile* AS_ProjectileWeapon::PerformProjectileSpawnLogic(
    const FVector& FireStartLocation,
    const FVector& FireDirection,
    const FGameplayEventData& EventData,
    TSubclassOf<AS_Projectile> ProjectileClass,
    float LaunchSpeed,
    float ProjectileLifeSpan)
{
    UE_LOG(LogTemp, Log, TEXT("AS_ProjectileWeapon::PerformProjectileSpawnLogic: %s - Start: %s, Dir: %s, ProjClass: %s, Speed: %f, Lifespan: %f. HasAuthority: %d"),
        *GetNameSafe(this), *FireStartLocation.ToString(), *FireDirection.ToString(), *GetNameSafe(ProjectileClass), LaunchSpeed, ProjectileLifeSpan, HasAuthority());

    if (!HasAuthority() || !OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_ProjectileWeapon::PerformProjectileSpawnLogic: %s - Authority check failed or OwnerCharacter is null."), *GetNameSafe(this));
        return nullptr;
    }

    AController* InstigatorController = OwnerCharacter->GetController();
    if (!InstigatorController)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_ProjectileWeapon::PerformProjectileSpawnLogic: %s - InstigatorController is null."), *GetNameSafe(this));
        return nullptr;
    }

    if (!ProjectileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_ProjectileWeapon::PerformProjectileSpawnLogic: %s - No ProjectileClass specified."), *GetNameSafe(this));
        return nullptr;
    }

    UWorld* const World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_ProjectileWeapon::PerformProjectileSpawnLogic: %s - GetWorld() returned null."), *GetNameSafe(this));
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this; // The weapon is the owner of the projectile
    SpawnParams.Instigator = OwnerCharacter; // The character is the instigator of the damage
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    FTransform SpawnTransform(FireDirection.Rotation(), FireStartLocation);
    UE_LOG(LogTemp, Verbose, TEXT("AS_ProjectileWeapon::PerformProjectileSpawnLogic: %s - Spawning projectile %s at Transform: %s"),
        *GetNameSafe(this), *ProjectileClass->GetName(), *SpawnTransform.ToString());

    AS_Projectile* SpawnedProjectile = World->SpawnActor<AS_Projectile>(ProjectileClass, SpawnTransform, SpawnParams);

    if (SpawnedProjectile)
    {
        UE_LOG(LogTemp, Log, TEXT("AS_ProjectileWeapon::PerformProjectileSpawnLogic: %s - Successfully spawned projectile %s."), *GetNameSafe(this), *SpawnedProjectile->GetName());

        // Pass the WeaponDataAsset for the projectile to use (e.g., for explosion cues)
        const US_ProjectileWeaponDataAsset* ProjWeaponData = Cast<US_ProjectileWeaponDataAsset>(GetWeaponData());
        SpawnedProjectile->InitializeProjectile(OwnerCharacter, this, ProjWeaponData);

        if (LaunchSpeed > 0.f && SpawnedProjectile->ProjectileMovementComponent)
        {
            SpawnedProjectile->ProjectileMovementComponent->InitialSpeed = LaunchSpeed;
            SpawnedProjectile->ProjectileMovementComponent->MaxSpeed = LaunchSpeed;
            UE_LOG(LogTemp, Verbose, TEXT("AS_ProjectileWeapon::PerformProjectileSpawnLogic: %s - Set projectile %s speed to %f."), *GetNameSafe(this), *SpawnedProjectile->GetName(), LaunchSpeed);
        }

        if (ProjectileLifeSpan > 0.f)
        {
            SpawnedProjectile->SetLifeSpan(ProjectileLifeSpan);
            UE_LOG(LogTemp, Verbose, TEXT("AS_ProjectileWeapon::PerformProjectileSpawnLogic: %s - Set projectile %s lifespan to %f."), *GetNameSafe(this), *SpawnedProjectile->GetName(), ProjectileLifeSpan);
        }
        RegisterProjectile(SpawnedProjectile);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_ProjectileWeapon::PerformProjectileSpawnLogic: %s - Failed to spawn projectile of class %s."), *GetNameSafe(this), *ProjectileClass->GetName());
    }
    return SpawnedProjectile;
}

void AS_ProjectileWeapon::RegisterProjectile(AS_Projectile* Projectile)
{
    if (HasAuthority() && Projectile && !ActiveProjectiles.Contains(Projectile))
    {
        ActiveProjectiles.Add(Projectile);
        Projectile->OnDestroyed.AddUniqueDynamic(this, &AS_ProjectileWeapon::OnProjectileDestroyed);
        UE_LOG(LogTemp, Verbose, TEXT("AS_ProjectileWeapon::RegisterProjectile: %s - Registered projectile %s. ActiveProjectiles: %d"), *GetNameSafe(this), *Projectile->GetName(), ActiveProjectiles.Num());
    }
}

void AS_ProjectileWeapon::UnregisterProjectile(AS_Projectile* Projectile)
{
    if (HasAuthority() && Projectile)
    {
        ActiveProjectiles.Remove(Projectile); // Remove by pointer
        UE_LOG(LogTemp, Verbose, TEXT("AS_ProjectileWeapon::UnregisterProjectile: %s - Unregistered projectile %s. ActiveProjectiles: %d"), *GetNameSafe(this), *Projectile->GetName(), ActiveProjectiles.Num());
    }
}

void AS_ProjectileWeapon::OnProjectileDestroyed(AActor* DestroyedActor)
{
    if (HasAuthority())
    {
        AS_Projectile* Projectile = Cast<AS_Projectile>(DestroyedActor);
        if (Projectile)
        {
            UE_LOG(LogTemp, Verbose, TEXT("AS_ProjectileWeapon::OnProjectileDestroyed: %s - Projectile %s was destroyed. Removing from active list."), *GetNameSafe(this), *Projectile->GetName());
            ActiveProjectiles.Remove(Projectile);
        }
    }
}