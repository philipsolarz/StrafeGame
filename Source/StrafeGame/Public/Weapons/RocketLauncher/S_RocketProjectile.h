#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_Projectile.h" // Inherits from base Projectile
#include "S_RocketProjectile.generated.h"

UCLASS(Blueprintable)
class STRAFEGAME_API AS_RocketProjectile : public AS_Projectile
{
    GENERATED_BODY()

public:
    AS_RocketProjectile();

protected:
    //~ Begin AActor Interface
    virtual void BeginPlay() override;
    //~ End AActor Interface

    //~ Begin AS_Projectile Interface
    /** Override if rockets have unique impact behavior before detonation. */
    // virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector HitNormal, const FHitResult& HitResult) override;
    /** Override for unique rocket explosion effects (though GameplayCue is preferred). */
    // virtual void K2_OnExplosionEffects(const FVector& ExplosionLocation) override;
    //~ End AS_Projectile Interface

    // Rocket-specific properties can be added here.
    // For example, a trail particle system component.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
    TObjectPtr<UParticleSystemComponent> TrailPSC; // Example for a particle trail
};