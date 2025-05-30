#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeProjectile.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h" // For CollisionComponent
#include "GameFramework/Character.h"    // To check if hit actor is a character
#include "Kismet/GameplayStatics.h"     // For potential effects

AS_StickyGrenadeProjectile::AS_StickyGrenadeProjectile()
{
    bIsStuck = false;
    bCanStickToCharacters = true; // Default, can be changed in BP
    StickyAttachmentOffset = 2.0f;

    // Sticky grenades usually don't explode on initial impact, but wait for manual detonation or lifespan.
    bExplodeOnImpact = false;
    MaxLifetime = 30.0f; // Stickies might last longer on the map

    if (ProjectileMovementComponent)
    {
        ProjectileMovementComponent->bShouldBounce = true; // They might bounce a bit before sticking or if they hit an invalid surface
        ProjectileMovementComponent->Bounciness = 0.3f;
        ProjectileMovementComponent->ProjectileGravityScale = 0.5f; // Give them some gravity
        ProjectileMovementComponent->InitialSpeed = 1500.f;
        ProjectileMovementComponent->MaxSpeed = 1500.f;
    }
    if (CollisionComponent)
    {
        // Ensure it generates hit events
        CollisionComponent->SetNotifyRigidBodyCollision(true);
    }
}

void AS_StickyGrenadeProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(AS_StickyGrenadeProjectile, bIsStuck, COND_None);
}

void AS_StickyGrenadeProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector HitNormal, const FHitResult& HitResult)
{
    // K2_OnImpact is called by base AS_Projectile::OnHit, but sticky needs to decide to stick *before* that.
    // However, for visual/audio on any hit *before* sticking, calling K2_OnImpact early might be desired here.
    // For now, let's assume K2_OnStuckToSurfaceEffects is the primary BP hook for successful sticking.
    // If general impact effects are needed even on non-sticking bounces, that can be added.

    if (!HasAuthority() || bIsStuck)
    {
        if (bIsStuck && ProjectileMovementComponent && ProjectileMovementComponent->IsActive())
        {
            ProjectileMovementComponent->StopMovementImmediately();
            ProjectileMovementComponent->Velocity = FVector::ZeroVector;
        }
        // Do not call Super::OnHit if already stuck, as that might try to detonate or destroy.
        return;
    }

    // K2_OnImpact(HitResult); // Optionally call this for any impact before stick/bounce decision

    bool bShouldAttemptStick = true;
    if (Cast<ACharacter>(OtherActor) && !bCanStickToCharacters)
    {
        bShouldAttemptStick = false;
    }

    if (bShouldAttemptStick && OtherComp && OtherComp->IsSimulatingPhysics())
    {
        bIsStuck = true;
        OnRep_IsStuck();

        if (ProjectileMovementComponent)
        {
            ProjectileMovementComponent->StopMovementImmediately();
            ProjectileMovementComponent->Velocity = FVector::ZeroVector;
            ProjectileMovementComponent->SetUpdatedComponent(nullptr);
        }

        SetActorEnableCollision(true);

        FVector AttachLocation = HitResult.ImpactPoint + HitResult.ImpactNormal * StickyAttachmentOffset;
        FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
        AttachToComponent(OtherComp, AttachRules, HitResult.BoneName);

        K2_OnStuckToSurfaceEffects(HitResult);
        return; // Successfully stuck, do not proceed to Super::OnHit or other logic
    }
    else if (bShouldAttemptStick) // Hit something static or non-character we can stick to
    {
        bIsStuck = true;
        OnRep_IsStuck();
        if (ProjectileMovementComponent)
        {
            ProjectileMovementComponent->StopMovementImmediately();
            ProjectileMovementComponent->Velocity = FVector::ZeroVector;
        }
        SetActorLocation(HitResult.ImpactPoint + HitResult.ImpactNormal * StickyAttachmentOffset);
        CollisionComponent->SetSimulatePhysics(false);
        SetActorEnableCollision(true);

        K2_OnStuckToSurfaceEffects(HitResult);
        return; // Successfully stuck, do not proceed to Super::OnHit or other logic
    }

    // If it didn't stick (e.g., hit a character and bCanStickToCharacters is false, or other condition):
    // Do NOT call Super::OnHit().
    // Since bExplodeOnImpact is false and bShouldBounce is true for this projectile,
    // the ProjectileMovementComponent will handle the bounce automatically after this function returns
    // if a bounce is appropriate according to physics.
    // No further action is needed here for the "fail to stick" case.
    UE_LOG(LogTemp, Verbose, TEXT("AS_StickyGrenadeProjectile::OnHit: %s - Failed to stick. Will rely on bounce (bShouldBounce=%d) or lifespan."),
        *GetNameSafe(this), ProjectileMovementComponent ? ProjectileMovementComponent->bShouldBounce : -1);

    // Explicitly call K2_OnImpact if you want Blueprint effects for non-sticking impacts as well
    // K2_OnImpact(HitResult); 
}

void AS_StickyGrenadeProjectile::OnRep_IsStuck()
{
    if (bIsStuck)
    {
        if (ProjectileMovementComponent)
        {
            ProjectileMovementComponent->StopMovementImmediately();
            ProjectileMovementComponent->Velocity = FVector::ZeroVector;
        }
        // K2_OnStuckToSurfaceEffects is primarily for server-side initiation of effects or logic.
        // Clients can have their own Blueprint BeginPlay/OnRep_IsStuck logic for visual effects if needed.
    }
}