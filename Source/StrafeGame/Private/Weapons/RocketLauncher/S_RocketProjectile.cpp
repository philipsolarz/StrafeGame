#include "Weapons/RocketLauncher/S_RocketProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h" // For UParticleSystemComponent
#include "Kismet/GameplayStatics.h"          // For spawning emitters if not using components

AS_RocketProjectile::AS_RocketProjectile()
{
    // Configure defaults for a typical rocket
    if (ProjectileMovementComponent)
    {
        ProjectileMovementComponent->InitialSpeed = 2500.f;
        ProjectileMovementComponent->MaxSpeed = 2500.f;
        ProjectileMovementComponent->bRotationFollowsVelocity = true;
        ProjectileMovementComponent->bShouldBounce = false;
        ProjectileMovementComponent->ProjectileGravityScale = 0.f; // Slight gravity for an arc, or 0 for straight
        ProjectileMovementComponent->HomingAccelerationMagnitude = 0.f; // No homing by default
    }

    if (CollisionComponent)
    {
        CollisionComponent->SetSphereRadius(12.f); // Rockets are a bit larger
    }

    bExplodeOnImpact = true;
    MaxLifetime = 7.0f; // Rockets last a bit longer

    BaseDamage = 100.0f;
    MinimumDamage = 25.0f;
    DamageInnerRadius = 150.0f;
    DamageOuterRadius = 350.0f;

    // Trail Particle System Component (optional, can also be done in Blueprint)
    TrailPSC = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailPSC"));
    TrailPSC->SetupAttachment(RootComponent);
    // TrailPSC->bAutoActivate = true; // Set particle template in Blueprint
}

void AS_RocketProjectile::BeginPlay()
{
    Super::BeginPlay();
    // Activate trail particle if set
    // if (TrailPSC && TrailPSC->Template)
    // {
    //     TrailPSC->ActivateSystem(true);
    // }
}

// Example Override for OnHit if needed for special pre-detonation logic
/*
void AS_RocketProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector HitNormal, const FHitResult& HitResult)
{
    // Do rocket-specific things before calling base or detonating.
    // For example, if it's a dud rocket that doesn't explode on certain surfaces.
    Super::OnHit(HitComp, OtherActor, OtherComp, HitNormal, HitResult); // Will call Detonate if bExplodeOnImpact is true
}
*/