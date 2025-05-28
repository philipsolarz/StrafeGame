// Source/StrafeGame/Private/Weapons/S_ProjectileWeapon.cpp
#include "Weapons/S_ProjectileWeapon.h"
#include "Weapons/S_Projectile.h"         
#include "Player/S_Character.h"
#include "Weapons/S_ProjectileWeaponDataAsset.h"    
#include "Engine/World.h"                 
#include "GameFramework/Controller.h"     
#include "GameFramework/ProjectileMovementComponent.h"

AS_ProjectileWeapon::AS_ProjectileWeapon()
{
    UE_LOG(LogTemp, Log, TEXT("AS_ProjectileWeapon::AS_ProjectileWeapon: Constructor for %s"), *GetNameSafe(this));
}

void AS_ProjectileWeapon::ExecuteFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData, float HitscanSpread, float HitscanRange, TSubclassOf<AS_Projectile> ProjectileClassFromAbility)
{
    UE_LOG(LogTemp, Log, TEXT("AS_ProjectileWeapon::ExecuteFire_Implementation: %s - FireStart: %s, FireDir: %s, ProjFromAbility: %s. HasAuthority: %d"),
         *GetNameSafe(this), *FireStartLocation.ToString(), *FireDirection.ToString(), *GetNameSafe(ProjectileClassFromAbility), HasAuthority());

    if (!HasAuthority())
    {
        return;
    }

    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_ProjectileWeapon::ExecuteFire_Implementation: %s - OwnerCharacter is null."), *GetNameSafe(this));
        return;
    }

    AController* InstigatorController = OwnerCharacter->GetController();
    if (!InstigatorController)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_ProjectileWeapon::ExecuteFire_Implementation: %s - InstigatorController is null."), *GetNameSafe(this));
        return;
    }

    const US_ProjectileWeaponDataAsset* ProjWeaponDataAssetFromWeapon = Cast<US_ProjectileWeaponDataAsset>(GetWeaponData());
    TSubclassOf<AS_Projectile> ActualProjectileClass = ProjectileClassFromAbility;

    if (!ActualProjectileClass && ProjWeaponDataAssetFromWeapon)
    {
        ActualProjectileClass = ProjWeaponDataAssetFromWeapon->ProjectileClass;
        UE_LOG(LogTemp, Verbose, TEXT("AS_ProjectileWeapon::ExecuteFire_Implementation: %s - Using ProjectileClass from DA: %s"), *GetNameSafe(this), *GetNameSafe(ActualProjectileClass));
    }
    else if (ActualProjectileClass)
    {
        UE_LOG(LogTemp, Verbose, TEXT("AS_ProjectileWeapon::ExecuteFire_Implementation: %s - Using ProjectileClass from Ability: %s"), *GetNameSafe(this), *GetNameSafe(ActualProjectileClass));
    }


    if (!ActualProjectileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_ProjectileWeapon::ExecuteFire_Implementation: %s - No ProjectileClass specified (neither from ability nor DataAsset)."), *GetNameSafe(this));
        return;
    }

    FTransform SpawnTransform(FireDirection.Rotation(), FireStartLocation);
    UE_LOG(LogTemp, Verbose, TEXT("AS_ProjectileWeapon::ExecuteFire_Implementation: %s - Spawning projectile %s at Transform: %s"), 
         *GetNameSafe(this), *ActualProjectileClass->GetName(), *SpawnTransform.ToString());
    AS_Projectile* SpawnedProjectile = SpawnProjectile(ActualProjectileClass, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), OwnerCharacter, InstigatorController);

    if (SpawnedProjectile)
    {
        UE_LOG(LogTemp, Log, TEXT("AS_ProjectileWeapon::ExecuteFire_Implementation: %s - Successfully spawned projectile %s."), *GetNameSafe(this), *SpawnedProjectile->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_ProjectileWeapon::ExecuteFire_Implementation: %s - Failed to spawn projectile of class %s."), *GetNameSafe(this), *ActualProjectileClass->GetName());
    }
}

AS_Projectile* AS_ProjectileWeapon::SpawnProjectile(TSubclassOf<AS_Projectile> ProjectileClass, const FVector& SpawnLocation, const FRotator& SpawnRotation, AS_Character* InstigatorCharacter, AController* InstigatorController)
{
    UWorld* const World = GetWorld();
    if (!World || !ProjectileClass || !InstigatorCharacter || !InstigatorController)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_ProjectileWeapon::SpawnProjectile: Preconditions not met. World: %d, ProjClass: %d, InstigatorChar: %d, InstigatorCtrl: %d"),
             World != nullptr, ProjectileClass != nullptr, InstigatorCharacter != nullptr, InstigatorController != nullptr);
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = InstigatorCharacter;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    UE_LOG(LogTemp, Verbose, TEXT("AS_ProjectileWeapon::SpawnProjectile: Attempting to spawn %s for %s."), *ProjectileClass->GetName(), *GetNameSafe(InstigatorCharacter));
    AS_Projectile* Projectile = World->SpawnActor<AS_Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
    if (Projectile)
    {
        const US_ProjectileWeaponDataAsset* ProjWeaponData = Cast<US_ProjectileWeaponDataAsset>(GetWeaponData());
        UE_LOG(LogTemp, Verbose, TEXT("AS_ProjectileWeapon::SpawnProjectile: Spawned %s. Initializing with WeaponData: %s."), *Projectile->GetName(), *GetNameSafe(ProjWeaponData));
        Projectile->InitializeProjectile(OwnerCharacter, this, ProjWeaponData);

        if (ProjWeaponData)
        {
            if (ProjWeaponData->LaunchSpeed > 0.f && Projectile->ProjectileMovementComponent) {
                Projectile->ProjectileMovementComponent->InitialSpeed = ProjWeaponData->LaunchSpeed;
                Projectile->ProjectileMovementComponent->MaxSpeed = ProjWeaponData->LaunchSpeed;
                UE_LOG(LogTemp, Verbose, TEXT("AS_ProjectileWeapon::SpawnProjectile: %s - Set projectile speed to %f from DA."), *Projectile->GetName(), ProjWeaponData->LaunchSpeed);
            }
            if (ProjWeaponData->ProjectileLifeSpan > 0.f) {
                Projectile->SetLifeSpan(ProjWeaponData->ProjectileLifeSpan);
                UE_LOG(LogTemp, Verbose, TEXT("AS_ProjectileWeapon::SpawnProjectile: %s - Set projectile lifespan to %f from DA."), *Projectile->GetName(), ProjWeaponData->ProjectileLifeSpan);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_ProjectileWeapon::SpawnProjectile: WeaponData for %s is not a ProjectileWeaponDataAsset. Projectile %s might use internal defaults for speed/lifespan."), *GetNameSafe(this), *Projectile->GetName());
        }
        RegisterProjectile(Projectile);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_ProjectileWeapon::SpawnProjectile: World->SpawnActor failed for %s."), *ProjectileClass->GetName());
    }
    return Projectile;
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
        ActiveProjectiles.Remove(Projectile);
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