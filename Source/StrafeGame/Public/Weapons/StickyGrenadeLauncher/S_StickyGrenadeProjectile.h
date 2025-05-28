#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_Projectile.h"
#include "S_StickyGrenadeProjectile.generated.h"

UCLASS(Blueprintable)
class STRAFEGAME_API AS_StickyGrenadeProjectile : public AS_Projectile
{
    GENERATED_BODY()

public:
    AS_StickyGrenadeProjectile();

    //~ Begin AActor Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    //~ End AActor Interface

    //~ Begin AS_Projectile Interface
    /** Overrides OnHit to implement sticking logic. */
    virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector HitNormal, const FHitResult& HitResult) override;
    //~ End AS_Projectile Interface

    UFUNCTION(BlueprintPure, Category = "StickyGrenade")
    bool IsStuckToSurface() const { return bIsStuck; }

protected:
    UPROPERTY(Transient, ReplicatedUsing = OnRep_IsStuck)
    bool bIsStuck;

    UFUNCTION()
    void OnRep_IsStuck();

    /** Visual/audio effects for when the grenade sticks. */
    UFUNCTION(BlueprintImplementableEvent, Category = "StickyGrenade|Effects", meta = (DisplayName = "OnStuckToSurfaceEffects"))
    void K2_OnStuckToSurfaceEffects(const FHitResult& Hit);

    /** Should this grenade stick to characters? Configurable. */
    UPROPERTY(EditDefaultsOnly, Category = "StickyGrenade")
    bool bCanStickToCharacters;

    /** Offset from impact point when sticking. */
    UPROPERTY(EditDefaultsOnly, Category = "StickyGrenade")
    float StickyAttachmentOffset;
};