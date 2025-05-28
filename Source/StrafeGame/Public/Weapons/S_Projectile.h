#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h" // For FGameplayTag
#include "S_Projectile.generated.h"

// Forward Declarations
class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class AS_Weapon; // The weapon that fired this projectile
class AS_Character; // The character that fired this projectile
class US_WeaponDataAsset; // To access weapon-specific projectile data like explosion cues
class UParticleSystem; // For particle effects (can be UNiagaraSystem too)
class USoundBase;      // For sounds

/**
 * Base class for all projectiles.
 * Handles movement, collision, lifetime, and basic impact/detonation logic.
 * Specific projectile types (e.g., Rocket, Grenade) will derive from this.
 */
UCLASS(Abstract, Blueprintable, Config = Game) // Abstract as it's a base, Blueprintable for derived BPs
class STRAFEGAME_API AS_Projectile : public AActor
{
    GENERATED_BODY()

public:
    AS_Projectile();

    //~ Begin AActor Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void LifeSpanExpired() override; // Called when lifespan expires
    //~ End AActor Interface

    /** Collision component for the projectile. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> CollisionComponent;

    /** Projectile movement component to handle trajectory and speed. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

    /** Optional static mesh for the projectile's visual representation. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ProjectileMeshComponent;

    /**
     * Initializes the projectile after it has been spawned.
     * Called by the weapon that fired it.
     * @param InInstigatorPawn The Pawn that instigated this projectile (usually the character who fired).
     * @param InOwningWeapon The Weapon actor that fired this projectile.
     * @param InWeaponDataAsset The DataAsset of the owning weapon, for accessing shared fx, cues etc.
     */
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    virtual void InitializeProjectile(APawn* InInstigatorPawn, AS_Weapon* InOwningWeapon, const US_WeaponDataAsset* InWeaponDataAsset);

    /**
     * Handles the projectile's impact with something.
     * Server-authoritative.
     * @param HitComp The component that was hit.
     * @param OtherActor The other actor involved in the collision.
     * @param OtherComp The other component involved in the collision.
     * @param Hit Normal impulse from the hit.
     * @param HitResult Detailed information about the hit.
     */
    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector HitNormal, const FHitResult& HitResult);

    /**
     * Detonates the projectile, applying damage and effects.
     * Server-authoritative.
     */
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    virtual void Detonate();

protected:
    /** The Pawn that fired this projectile. Replicated. */
    UPROPERTY(Transient, Replicated)
    TObjectPtr<APawn> InstigatorPawn;

    /** The weapon that fired this projectile. Replicated. */
    UPROPERTY(Transient, Replicated)
    TObjectPtr<AS_Weapon> OwningWeapon;

    /** WeaponDataAsset from the owning weapon. Not replicated, set on spawn for server & client. */
    UPROPERTY(BlueprintReadOnly, Category = "ProjectileConfig")
    TObjectPtr<const US_WeaponDataAsset> OwningWeaponDataAsset;

    /** If true, the projectile will explode when it hits anything. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Behavior")
    bool bExplodeOnImpact;

    /** Maximum time this projectile will exist in the world before self-destructing. 0 means no lifespan. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Behavior", meta = (ClampMin = "0.0"))
    float MaxLifetime;

    // --- Damage & Effects (Configured in derived classes or via WeaponDataAsset) ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Damage")
    float BaseDamage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Damage")
    float MinimumDamage; // For radial damage falloff

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Damage")
    float DamageInnerRadius; // For radial damage

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Damage")
    float DamageOuterRadius; // For radial damage

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Damage")
    TSubclassOf<class UDamageType> DamageTypeClass;

    /** GameplayCue tag for the explosion effect. Should be defined in this projectile's BP or its weapon's DataAsset. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Effects", meta = (DisplayName = "Explosion Effect Cue"))
    FGameplayTag ExplosionCueTag;

    /** Server-side function to apply radial damage. */
    virtual void ApplyRadialDamage();

    /**
     * Plays explosion effects.
     * This is typically called on all clients via a GameplayCue, triggered by the server.
     * The GameplayCue itself will handle spawning particles, sounds, etc.
     * @param ExplosionLocation The location of the explosion.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|Effects", meta = (DisplayName = "OnExplosionEffects"))
    void K2_OnExplosionEffects(const FVector& ExplosionLocation);

    /**
     * Called when the projectile impacts something.
     * Can be overridden in Blueprints for custom impact logic before potential explosion.
     * @param HitResult Detailed information about the hit.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|Events", meta = (DisplayName = "OnImpact"))
    void K2_OnImpact(const FHitResult& HitResult);

    /** Called when the projectile is spawned and initialized. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|Events", meta = (DisplayName = "OnSpawned"))
    void K2_OnSpawned();

private:
    /** Server RPC to request detonation. Useful if detonation can be triggered by something other than impact. */
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RequestDetonation();
};