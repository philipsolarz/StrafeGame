#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h" // For FGameplayTag
#include "Components/SphereComponent.h"
#include "S_Projectile.generated.h"

// Forward Declarations
class UProjectileMovementComponent;
class UStaticMeshComponent;
class AS_ProjectileWeapon;
class AS_Character;
class US_ProjectileWeaponDataAsset;
class UParticleSystem;
class USoundBase;

/**
 * Base class for all projectiles.
 */
UCLASS(Abstract, Blueprintable, Config = Game)
class STRAFEGAME_API AS_Projectile : public AActor
{
	GENERATED_BODY()

public:
	AS_Projectile();

	//~ Begin AActor Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void LifeSpanExpired() override;
	//~ End AActor Interface

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ProjectileMeshComponent;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual void InitializeProjectile(APawn* InInstigatorPawn, AS_ProjectileWeapon* InOwningWeapon, const US_ProjectileWeaponDataAsset* InWeaponDataAsset);

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector HitNormal, const FHitResult& HitResult);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual void Detonate();

protected:
	UPROPERTY(Transient, Replicated)
	TObjectPtr<APawn> InstigatorPawn;

	UPROPERTY(Transient, Replicated)
	TObjectPtr<AS_ProjectileWeapon> OwningWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "ProjectileConfig")
	TObjectPtr<const US_ProjectileWeaponDataAsset> OwningWeaponDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Behavior")
	bool bExplodeOnImpact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Behavior", meta = (ClampMin = "0.0"))
	float MaxLifetime;

	// --- Damage & Effects ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Damage")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Damage")
	float MinimumDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Damage")
	float DamageInnerRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Damage")
	float DamageOuterRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Damage")
	TSubclassOf<class UDamageType> DamageTypeClass;

	// --- Knockback Configuration ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Knockback", meta = (Tooltip = "The launch speed (in cm/s) applied to characters. A good starting value is ~1500."))
	float KnockbackForce;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Knockback", meta = (Tooltip = "Adds a Z-axis velocity to the knockback, launching the character upwards."))
	bool bApplyUpwardKnockback;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Knockback", meta = (EditCondition = "bApplyUpwardKnockback", Tooltip = "The Z-axis launch speed (in cm/s) to add. A good starting value is ~750."))
	float UpwardKnockbackStrength;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileConfig|Effects", meta = (DisplayName = "Explosion Effect Cue"))
	FGameplayTag ExplosionCueTag;

	virtual void ApplyRadialDamage();

	virtual void ApplyRadialKnockback();

	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|Effects", meta = (DisplayName = "OnExplosionEffects"))
	void K2_OnExplosionEffects(const FVector& ExplosionLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|Events", meta = (DisplayName = "OnImpact"))
	void K2_OnImpact(const FHitResult& HitResult);

	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|Events", meta = (DisplayName = "OnSpawned"))
	void K2_OnSpawned();

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestDetonation();
};