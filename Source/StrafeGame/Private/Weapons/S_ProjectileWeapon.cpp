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

void AS_ProjectileWeapon::ExecuteFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData, float HitscanSpread, float HitscanRange, TSubclassOf<AS_Projectile> ProjectileClassFromAbility) // MODIFIED
{
    // Pass EventData along if Super uses it.
    // Super::ExecuteFire_Implementation(FireStartLocation, FireDirection, EventData, HitscanSpread, HitscanRange, ProjectileClassFromAbility); // MODIFIED

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

    const US_ProjectileWeaponDataAsset* ProjWeaponData = Cast<US_ProjectileWeaponDataAsset>(GetWeaponData());
    TSubclassOf<AS_Projectile> ActualProjectileClass = ProjectileClassFromAbility;

    if (!ActualProjectileClass && ProjWeaponData) // Fallback to DA if ability didn't provide (shouldn't happen with new design)
    {
        ActualProjectileClass = ProjWeaponData->ProjectileClass;
    }

    if (!ActualProjectileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_ProjectileWeapon::ExecuteFire: No ProjectileClass specified for %s."), *GetName());
        return;
    }

    FTransform SpawnTransform(FireDirection.Rotation(), FireStartLocation);
    // Use ProjWeaponData for LaunchSpeed and ProjectileLifeSpan when initializing the projectile
    AS_Projectile* SpawnedProjectile = SpawnProjectile(ActualProjectileClass, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), OwnerCharacter, InstigatorController);

    if (SpawnedProjectile)
    {
        // Initialization like speed, lifespan should happen in InitializeProjectile or be part of the projectile's defaults
        // based on data from its DataAsset or the weapon's DataAsset.
        // For example, the GameplayAbility would read these from the WeaponDataAsset and pass them somehow if they were dynamic per shot.
        // Or Projectile itself reads from its own DA or defaults.
        if (ProjWeaponData)
        {
            SpawnedProjectile->InitializeProjectile(OwnerCharacter, this, ProjWeaponData); // Pass weapon data
            if (ProjWeaponData->LaunchSpeed > 0.f && SpawnedProjectile->GetProjectileMovementComponent()) {
                SpawnedProjectile->GetProjectileMovementComponent()->InitialSpeed = ProjWeaponData->LaunchSpeed;
                SpawnedProjectile->GetProjectileMovementComponent()->MaxSpeed = ProjWeaponData->LaunchSpeed;
            }
            if (ProjWeaponData->ProjectileLifeSpan > 0.f) {
                SpawnedProjectile->SetLifeSpan(ProjWeaponData->ProjectileLifeSpan);
            }
        }
        else {
            SpawnedProjectile->InitializeProjectile(OwnerCharacter, this, GetWeaponData());
        }
    }
    // Muzzle flash cue should be triggered by the GameplayAbility
}

AS_Projectile* AS_ProjectileWeapon::SpawnProjectile(TSubclassOf<AS_Projectile> ProjectileClass, const FVector& SpawnLocation, const FRotator& SpawnRotation, AS_Character* InstigatorCharacter, AController* InstigatorController)
{
    UWorld* const World = GetWorld();
    if (!World || !ProjectileClass || !InstigatorCharacter || !InstigatorController)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this; // The weapon is the direct owner of the projectile
    SpawnParams.Instigator = InstigatorCharacter; // The character is the instigator of the damage/effects
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    //SpawnParams.bDeferConstruction = true; // If you need to set properties before BeginPlay/Construction

    AS_Projectile* Projectile = World->SpawnActor<AS_Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
    if (Projectile)
    {
        // Projectile->SetProjectileWeapon(this); // Let projectile know its weapon
        // Projectile->SetInstigatorController(InstigatorController);
        // Projectile->SetInstigatorCharacter(InstigatorCharacter);
        // Projectile->FinishSpawning(FTransform(SpawnRotation, SpawnLocation)); // If bDeferConstruction was true

        RegisterProjectile(Projectile); // Track it if needed
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