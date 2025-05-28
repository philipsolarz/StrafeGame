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
    if (!HasAuthority() || bIsStuck) // Only stick once, server authoritative
    {
        if (bIsStuck && ProjectileMovementComponent && ProjectileMovementComponent->IsActive())
        {
            // If already stuck but somehow still moving, stop it hard.
            ProjectileMovementComponent->StopMovementImmediately();
            ProjectileMovementComponent->Velocity = FVector::ZeroVector;
        }
        // Don't call Super::OnHit if already stuck, as that might try to detonate or destroy.
        return;
    }

    bool bShouldAttemptStick = true;
    if (Cast<ACharacter>(OtherActor) && !bCanStickToCharacters)
    {
        bShouldAttemptStick = false;
    }

    if (bShouldAttemptStick && OtherComp && OtherComp->IsSimulatingPhysics()) // Stick to physics objects
    {
        // Stick to surface
        bIsStuck = true;
        OnRep_IsStuck(); // Call OnRep manually on server

        if (ProjectileMovementComponent)
        {
            ProjectileMovementComponent->StopMovementImmediately();
            ProjectileMovementComponent->Velocity = FVector::ZeroVector;
            ProjectileMovementComponent->SetUpdatedComponent(nullptr); // Stop trying to move
        }

        SetActorEnableCollision(true); // Keep collision for detonation radius checks, but it's not moving

        // Attach with offset
        FVector AttachLocation = HitResult.ImpactPoint + HitResult.ImpactNormal * StickyAttachmentOffset;
        FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
        // We want to attach to the component that was hit, maintaining world position initially, then let physics handle if it's a simulating component
        AttachToComponent(OtherComp, AttachRules, HitResult.BoneName);
        // After attaching, we might want to adjust to keep the world transform to avoid snapping if OtherComp moves violently *during* attach
        // For simplicity, KeepWorld on attach might be fine, or a Weld. Test what looks best.
        // If OtherComp is static, this is simpler.

        K2_OnStuckToSurfaceEffects(HitResult); // Call BP event for sticking effects

        // Do NOT call Super::OnHit() here if sticking, because Super::OnHit might call Detonate() if bExplodeOnImpact is true
        // (which it is for base AS_Projectile, but we set it false for Sticky in its constructor).
        // If we wanted it to explode on impact IF IT FAILS TO STICK, then we'd call super.
        return;
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
        // Make it "kinematic" but still collidable for detonation
        CollisionComponent->SetSimulatePhysics(false);
        SetActorEnableCollision(true);

        K2_OnStuckToSurfaceEffects(HitResult);
        return;
    }

    // If it didn't stick (e.g., hit a character and bCanStickToCharacters is false, or other condition)
    // then let the default projectile OnHit logic (bounce, etc.) from AS_Projectile potentially take over,
    // or just destroy it if that's preferred for non-sticking impacts.
    // Since bExplodeOnImpact is false for stickies, Super::OnHit won't explode it.
    // It might try to bounce if ProjectileMovementComponent is configured for it.
    Super::OnHit(HitComp, OtherActor, OtherComp, HitNormal, HitResult);
}

void AS_StickyGrenadeProjectile::OnRep_IsStuck()
{
    if (bIsStuck)
    {
        // Client-side reaction to sticking
        if (ProjectileMovementComponent)
        {
            ProjectileMovementComponent->StopMovementImmediately(); // Ensure client also stops movement
            ProjectileMovementComponent->Velocity = FVector::ZeroVector;
        }
        // K2_OnStuckToSurfaceEffects can be called here too if needed for client-side effect initiation
        // but usually the attachment replication handles the visual "sticking".
        // It's mainly for server logic and immediate server-side BP effects.
    }
}