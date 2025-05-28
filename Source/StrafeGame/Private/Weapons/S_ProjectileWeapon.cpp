#include "Weapons/S_ProjectileWeapon.h"
#include "Weapons/S_Projectile.h"         // For AS_Projectile
#include "Player/S_Character.h"
#include "Weapons/S_WeaponDataAsset.h"    // For getting projectile class
#include "Engine/World.h"                 // For spawning
#include "GameFramework/Controller.h"     // For AController

AS_ProjectileWeapon::AS_ProjectileWeapon()
{
    // DefaultProjectileClass = nullptr; // To be set in derived BPs or read from DataAsset by GA
}

void AS_ProjectileWeapon::ExecuteFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData* EventData, float HitscanSpread, float HitscanRange, TSubclassOf<AS_Projectile> ProjectileClassFromAbility)
{
    Super::ExecuteFire_Implementation(FireStartLocation, FireDirection, EventData, HitscanSpread, HitscanRange, ProjectileClassFromAbility);

    if (GetOwnerRole() != ROLE_Authority) // Server spawns projectiles
    {
        // Client might play cosmetic muzzle flash here if not handled by ability's gameplay cue
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

    TSubclassOf<AS_Projectile> ActualProjectileClass = ProjectileClassFromAbility;
    // Fallback to a default from WeaponDataAsset if ability didn't provide one (though ability should be authoritative)
    if (!ActualProjectileClass && WeaponData && WeaponData->PrimaryProjectileClass_DEPRECATED) // Assuming direct TSubclassOf on WeaponData for simplicity here
    {
        ActualProjectileClass = WeaponData->PrimaryProjectileClass_DEPRECATED;
    }

    if (!ActualProjectileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_ProjectileWeapon::ExecuteFire: No ProjectileClass specified for %s."), *GetName());
        return;
    }

    FTransform SpawnTransform(FireDirection.Rotation(), FireStartLocation);
    AS_Projectile* SpawnedProjectile = SpawnProjectile(ActualProjectileClass, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), OwnerCharacter, InstigatorController);

    if (SpawnedProjectile)
    {
        // UE_LOG(LogTemp, Log, TEXT("AS_ProjectileWeapon %s fired projectile %s"), *GetName(), *SpawnedProjectile->GetName());
    }
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