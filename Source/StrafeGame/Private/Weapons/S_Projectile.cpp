#include "Weapons/S_Projectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Weapons/S_ProjectileWeapon.h"
#include "Player/S_Character.h"
#include "Weapons/S_ProjectileWeaponDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

AS_Projectile::AS_Projectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->InitSphereRadius(10.0f);
	CollisionComponent->bReturnMaterialOnMove = true;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
	ProjectileMovementComponent->InitialSpeed = 3000.f;
	ProjectileMovementComponent->MaxSpeed = 3000.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

	ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMeshComponent->SetupAttachment(CollisionComponent);
	ProjectileMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bExplodeOnImpact = true;
	MaxLifetime = 5.0f;
	BaseDamage = 50.0f;
	MinimumDamage = 10.0f;
	DamageInnerRadius = 100.0f;
	DamageOuterRadius = 300.0f;
	DamageTypeClass = UDamageType::StaticClass();

	// UPDATED: Default values are now launch speeds, not abstract forces.
	KnockbackForce = 1500.f;
	bApplyUpwardKnockback = true;
	UpwardKnockbackStrength = 750.f;

	InstigatorPawn = nullptr;
	OwningWeapon = nullptr;
	OwningWeaponDataAsset = nullptr;
}

// ... (GetLifetimeReplicatedProps, BeginPlay, EndPlay, etc. are unchanged) ...

void AS_Projectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AS_Projectile, InstigatorPawn);
	DOREPLIFETIME(AS_Projectile, OwningWeapon);
}

void AS_Projectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &AS_Projectile::OnHit);
	}

	if (MaxLifetime > 0.0f)
	{
		SetLifeSpan(MaxLifetime);
	}

	K2_OnSpawned();
}

void AS_Projectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OwningWeapon && HasAuthority())
	{
		if (AS_ProjectileWeapon* ProjWeapon = Cast<AS_ProjectileWeapon>(OwningWeapon))
		{
			ProjWeapon->UnregisterProjectile(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}


void AS_Projectile::LifeSpanExpired()
{
	if (HasAuthority())
	{
		if (bExplodeOnImpact)
		{
			Detonate();
		}
		else
		{
			Destroy();
		}
	}
}

void AS_Projectile::InitializeProjectile(APawn* InInstigatorPawn, AS_ProjectileWeapon* InOwningWeapon, const US_ProjectileWeaponDataAsset* InWeaponDataAsset)
{
	InstigatorPawn = InInstigatorPawn;
	SetInstigator(InInstigatorPawn);
	OwningWeapon = InOwningWeapon;
	OwningWeaponDataAsset = InWeaponDataAsset;

	if (InInstigatorPawn)
	{
		SetOwner(InInstigatorPawn->GetController());
		if (CollisionComponent)
		{
			CollisionComponent->IgnoreActorWhenMoving(InInstigatorPawn, true);
		}
	}
	else if (InOwningWeapon)
	{
		SetOwner(InOwningWeapon->GetOwner());
	}

	if (InOwningWeapon && CollisionComponent)
	{
		CollisionComponent->IgnoreActorWhenMoving(InOwningWeapon, true);
	}


	if (OwningWeaponDataAsset)
	{
		if (OwningWeaponDataAsset->LaunchSpeed > 0 && ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = OwningWeaponDataAsset->LaunchSpeed;
			ProjectileMovementComponent->MaxSpeed = OwningWeaponDataAsset->LaunchSpeed;
		}
		if (OwningWeaponDataAsset->ProjectileLifeSpan > 0) MaxLifetime = OwningWeaponDataAsset->ProjectileLifeSpan;
		if (OwningWeaponDataAsset->ProjectileExplosionCueTag.IsValid()) ExplosionCueTag = OwningWeaponDataAsset->ProjectileExplosionCueTag;
	}
}

void AS_Projectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector HitNormal, const FHitResult& HitResult)
{
	if (!HasAuthority()) return;

	K2_OnImpact(HitResult);

	if (bExplodeOnImpact)
	{
		Detonate();
	}
	else
	{
		if (ProjectileMovementComponent && !ProjectileMovementComponent->bShouldBounce)
		{
			Destroy();
		}
	}
}

void AS_Projectile::Detonate()
{
	if (!HasAuthority())
	{
		Server_RequestDetonation();
		return;
	}

#if ENABLE_DRAW_DEBUG
	DrawDebugSphere(GetWorld(), GetActorLocation(), DamageOuterRadius, 26, FColor::Red, false, 2.0f, 0, 1.0f);
#endif

	ApplyRadialDamage();
	ApplyRadialKnockback();

	FGameplayTag CueToPlay = ExplosionCueTag;
	if (OwningWeaponDataAsset && OwningWeaponDataAsset->ProjectileExplosionCueTag.IsValid())
	{
		CueToPlay = OwningWeaponDataAsset->ProjectileExplosionCueTag;
	}


	if (CueToPlay.IsValid() && InstigatorPawn)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstigatorPawn);
		if (ASC)
		{
			FGameplayCueParameters CueParams;
			CueParams.Location = GetActorLocation();
			CueParams.Instigator = InstigatorPawn;
			CueParams.EffectCauser = this;
			ASC->ExecuteGameplayCue(CueToPlay, CueParams);
		}
	}
	else
	{
		K2_OnExplosionEffects(GetActorLocation());
	}

	if (OwningWeapon)
	{
		if (AS_ProjectileWeapon* ProjWeapon = Cast<AS_ProjectileWeapon>(OwningWeapon))
		{
			ProjWeapon->UnregisterProjectile(this);
		}
	}

	Destroy();
}

void AS_Projectile::ApplyRadialDamage()
{
	if (!HasAuthority()) return;

	AController* EventInstigatorController = nullptr;
	if (InstigatorPawn)
	{
		EventInstigatorController = InstigatorPawn->GetController();
	}

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);
	if (InstigatorPawn) IgnoredActors.Add(InstigatorPawn);
	if (OwningWeapon) IgnoredActors.Add(OwningWeapon);

	UGameplayStatics::ApplyRadialDamageWithFalloff(
		this,
		BaseDamage,
		MinimumDamage,
		GetActorLocation(),
		DamageInnerRadius,
		DamageOuterRadius,
		1.0f,
		DamageTypeClass,
		IgnoredActors,
		this,
		EventInstigatorController
	);
}

void AS_Projectile::ApplyRadialKnockback()
{
	if (!HasAuthority() || KnockbackForce <= 0.f) return;

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);
	if (InstigatorPawn) IgnoredActors.Add(InstigatorPawn);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		GetActorLocation(),
		DamageOuterRadius,
		ObjectTypes,
		AS_Character::StaticClass(),
		IgnoredActors,
		OverlappedActors
	);

	for (AActor* OverlappedActor : OverlappedActors)
	{
		// We use the base ACharacter class here because LaunchCharacter is a function on it
		ACharacter* CharacterToLaunch = Cast<ACharacter>(OverlappedActor);
		if (CharacterToLaunch)
		{
			// Calculate the direction from the explosion to the character
			FVector LaunchDirection = CharacterToLaunch->GetActorLocation() - GetActorLocation();

			// We only want the horizontal direction for the main knockback force
			LaunchDirection.Z = 0;
			LaunchDirection.Normalize();

			// Calculate falloff based on distance
			const float Distance = FVector::Dist(GetActorLocation(), CharacterToLaunch->GetActorLocation());
			const float Falloff = 1.0f - FMath::Clamp(Distance / DamageOuterRadius, 0.0f, 1.0f);

			// Calculate the final launch velocity vector
			FVector FinalLaunchVelocity = LaunchDirection * KnockbackForce * Falloff;

			// Add the upward knockback strength, also affected by falloff
			if (bApplyUpwardKnockback)
			{
				FinalLaunchVelocity.Z = UpwardKnockbackStrength * Falloff;
			}

			// Launch the character! 
			// bXYOverride=true replaces the existing horizontal velocity.
			// bZOverride=true replaces the existing vertical velocity.
			CharacterToLaunch->LaunchCharacter(FinalLaunchVelocity, true, true);

			UE_LOG(LogTemp, Log, TEXT("AS_Projectile::ApplyRadialKnockback: Launched %s with velocity %s"), *CharacterToLaunch->GetName(), *FinalLaunchVelocity.ToString());
		}
	}
}

bool AS_Projectile::Server_RequestDetonation_Validate() { return true; }
void AS_Projectile::Server_RequestDetonation_Implementation()
{
	Detonate();
}