// Source/StrafeGame/Private/Weapons/S_Projectile.cpp
#include "Weapons/S_Projectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Weapons/S_ProjectileWeapon.h"
#include "Player/S_Character.h" 
#include "Weapons/S_ProjectileWeaponDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h" 
#include "GameFramework/DamageType.h" 
#include "AbilitySystemBlueprintLibrary.h" 
#include "AbilitySystemComponent.h"    

AS_Projectile::AS_Projectile()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    RootComponent = CollisionComponent;
    CollisionComponent->InitSphereRadius(10.0f);
    //CollisionComponent->SetCollisionProfileName(TEXT("Projectile")); // Ensure this profile is set up correctly in project settings
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

    InstigatorPawn = nullptr;
    OwningWeapon = nullptr;
    OwningWeaponDataAsset = nullptr;
    UE_LOG(LogTemp, Log, TEXT("AS_Projectile::AS_Projectile: Constructor for %s"), *GetNameSafe(this));
}

void AS_Projectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AS_Projectile, InstigatorPawn);
    DOREPLIFETIME(AS_Projectile, OwningWeapon);
}

void AS_Projectile::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("AS_Projectile::BeginPlay: %s - Instigator: %s, OwningWeapon: %s. HasAuthority: %d"),
        *GetNameSafe(this), *GetNameSafe(InstigatorPawn), *GetNameSafe(OwningWeapon), HasAuthority());

    if (HasAuthority())
    {
        CollisionComponent->OnComponentHit.AddDynamic(this, &AS_Projectile::OnHit);
        UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::BeginPlay: %s - Bound OnHit delegate (Server)."), *GetNameSafe(this));
    }

    if (MaxLifetime > 0.0f)
    {
        SetLifeSpan(MaxLifetime);
        UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::BeginPlay: %s - Lifespan set to %f seconds."), *GetNameSafe(this), MaxLifetime);
    }

    K2_OnSpawned();
}

void AS_Projectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (OwningWeapon && HasAuthority())
    {
        if (AS_ProjectileWeapon* ProjWeapon = Cast<AS_ProjectileWeapon>(OwningWeapon))
        {
            UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::EndPlay: %s - Unregistering from OwningWeapon %s."), *GetNameSafe(this), *ProjWeapon->GetName());
            ProjWeapon->UnregisterProjectile(this);
        }
    }
    Super::EndPlay(EndPlayReason);
}


void AS_Projectile::LifeSpanExpired()
{
    UE_LOG(LogTemp, Log, TEXT("AS_Projectile::LifeSpanExpired: %s. HasAuthority: %d, bExplodeOnImpact: %d"), *GetNameSafe(this), HasAuthority(), bExplodeOnImpact);
    if (HasAuthority())
    {
        if (bExplodeOnImpact)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_Projectile %s: Lifespan expired, detonating."), *GetName());
            Detonate();
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("AS_Projectile %s: Lifespan expired, destroying without detonation."), *GetName());
            Destroy();
        }
    }
}

void AS_Projectile::InitializeProjectile(APawn* InInstigatorPawn, AS_ProjectileWeapon* InOwningWeapon, const US_ProjectileWeaponDataAsset* InWeaponDataAsset)
{
    InstigatorPawn = InInstigatorPawn;
    SetInstigator(InInstigatorPawn); // Sets the instigator for damage dealing and other systems
    OwningWeapon = InOwningWeapon;
    OwningWeaponDataAsset = InWeaponDataAsset;

    UE_LOG(LogTemp, Log, TEXT("AS_Projectile::InitializeProjectile: %s - Instigator: %s, Weapon: %s, WeaponDA: %s"),
        *GetNameSafe(this), *GetNameSafe(InInstigatorPawn), *GetNameSafe(InOwningWeapon), *GetNameSafe(InWeaponDataAsset));

    if (InInstigatorPawn)
    {
        SetOwner(InInstigatorPawn->GetController()); // Projectile is "owned" by the controller of the instigator
        if (CollisionComponent)
        {
            // Make the projectile ignore collision with the pawn that fired it.
            CollisionComponent->IgnoreActorWhenMoving(InInstigatorPawn, true);
            UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::InitializeProjectile: %s - Ignoring collision with InstigatorPawn: %s"), *GetNameSafe(this), *InInstigatorPawn->GetName());
        }
    }
    else if (InOwningWeapon)
    {
        SetOwner(InOwningWeapon->GetOwner()); // Fallback to weapon's owner if instigator pawn is somehow null
    }

    if (InOwningWeapon && CollisionComponent)
    {
        // Also ignore the weapon actor itself.
        CollisionComponent->IgnoreActorWhenMoving(InOwningWeapon, true);
        UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::InitializeProjectile: %s - Ignoring collision with OwningWeapon: %s"), *GetNameSafe(this), *InOwningWeapon->GetName());
    }


    if (OwningWeaponDataAsset)
    {
        UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::InitializeProjectile: %s - Applying settings from DA: LaunchSpeed: %f, Lifespan: %f"),
            *GetNameSafe(this), OwningWeaponDataAsset->LaunchSpeed, OwningWeaponDataAsset->ProjectileLifeSpan);
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
    UE_LOG(LogTemp, Log, TEXT("AS_Projectile::OnHit: %s - Hit %s (Comp: %s). HasAuthority: %d, bExplodeOnImpact: %d"),
        *GetNameSafe(this), *GetNameSafe(OtherActor), *GetNameSafe(OtherComp), HasAuthority(), bExplodeOnImpact);
    if (!HasAuthority()) return;

    K2_OnImpact(HitResult); // Call BP event for cosmetic effects or special early logic

    if (bExplodeOnImpact)
    {
        UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::OnHit: %s - Exploding on impact."), *GetNameSafe(this));
        Detonate();
    }
    else
    {
        // If the projectile is not set to explode on impact,
        // its behavior is primarily determined by its ProjectileMovementComponent (e.g., bShouldBounce).
        // If it's not set to bounce by default, and has no other special behavior defined in a derived class
        // that would return before this, destroy it.
        if (ProjectileMovementComponent && ProjectileMovementComponent->bShouldBounce)
        {
            UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::OnHit: %s - Not exploding, configured to bounce. Letting movement component handle it."), *GetNameSafe(this));
            // The ProjectileMovementComponent will handle the bounce if bShouldBounce is true.
            // No explicit Destroy() here if it's meant to bounce.
        }
        else
        {
            // Not exploding, not configured to bounce by default in this base class.
            UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::OnHit: %s - Not exploding, not configured to bounce by default. Destroying."), *GetNameSafe(this));
            Destroy();
        }
    }
}

void AS_Projectile::Detonate()
{
    UE_LOG(LogTemp, Log, TEXT("AS_Projectile::Detonate: %s. HasAuthority: %d"), *GetNameSafe(this), HasAuthority());
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::Detonate: %s - Client requesting server detonation."), *GetNameSafe(this));
        Server_RequestDetonation();
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AS_Projectile %s: DETONATING at %s."), *GetName(), *GetActorLocation().ToString());

    ApplyRadialDamage();

    FGameplayTag CueToPlay = ExplosionCueTag;
    if (OwningWeaponDataAsset && OwningWeaponDataAsset->ProjectileExplosionCueTag.IsValid())
    {
        CueToPlay = OwningWeaponDataAsset->ProjectileExplosionCueTag;
        UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::Detonate: %s - Using ExplosionCueTag from WeaponDA: %s"), *GetNameSafe(this), *CueToPlay.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::Detonate: %s - Using local ExplosionCueTag: %s"), *GetNameSafe(this), *CueToPlay.ToString());
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
            UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::Detonate: %s - Executed GameplayCue %s via Instigator ASC."), *GetNameSafe(this), *CueToPlay.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_Projectile::Detonate: %s - Instigator %s has no ASC. Cannot play cue %s."), *GetNameSafe(this), *GetNameSafe(InstigatorPawn), *CueToPlay.ToString());
        }
    }
    else {
        UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::Detonate: %s - No valid cue (%s) or InstigatorPawn (%s). Calling K2_OnExplosionEffects."), *GetNameSafe(this), *CueToPlay.ToString(), *GetNameSafe(InstigatorPawn));
        K2_OnExplosionEffects(GetActorLocation());
    }

    if (OwningWeapon)
    {
        if (AS_ProjectileWeapon* ProjWeapon = Cast<AS_ProjectileWeapon>(OwningWeapon))
        {
            UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::Detonate: %s - Unregistering self from OwningWeapon %s prior to destroy."), *GetNameSafe(this), *ProjWeapon->GetName());
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
    IgnoredActors.Add(this); // Projectile shouldn't damage itself with its own explosion
    if (InstigatorPawn) IgnoredActors.Add(InstigatorPawn);
    if (OwningWeapon) IgnoredActors.Add(OwningWeapon);

    UE_LOG(LogTemp, Log, TEXT("AS_Projectile::ApplyRadialDamage: %s - Applying radial damage. Base: %f, Min: %f, InnerR: %f, OuterR: %f, Location: %s"),
        *GetNameSafe(this), BaseDamage, MinimumDamage, DamageInnerRadius, DamageOuterRadius, *GetActorLocation().ToString());

    UGameplayStatics::ApplyRadialDamageWithFalloff(
        this, // DamageCauser Context
        BaseDamage,
        MinimumDamage,
        GetActorLocation(),
        DamageInnerRadius,
        DamageOuterRadius,
        1.0f, // DamageFalloff exponent
        DamageTypeClass,
        IgnoredActors, // Actors to ignore for this damage application
        this,          // DamageCauser (the projectile itself)
        EventInstigatorController // Controller that instigated the damage
    );

    // Debug visuals
    if (GetWorld() && (GetWorld()->IsNetMode(NM_ListenServer) || GetWorld()->IsNetMode(NM_Standalone)))
    {
        DrawDebugSphere(
            GetWorld(),
            GetActorLocation(),
            DamageOuterRadius,
            32,
            FColor::Red,
            false,
            5.0f,
            0,
            1.0f
        );
        DrawDebugSphere(
            GetWorld(),
            GetActorLocation(),
            DamageInnerRadius,
            32,
            FColor::Yellow,
            false,
            5.0f,
            0,
            1.0f
        );
    }
}

bool AS_Projectile::Server_RequestDetonation_Validate() { return true; }
void AS_Projectile::Server_RequestDetonation_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("AS_Projectile::Server_RequestDetonation_Implementation: %s - Server received detonation request."), *GetNameSafe(this));
    Detonate();
}