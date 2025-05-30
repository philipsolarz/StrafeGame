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
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
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
    SetInstigator(InInstigatorPawn);
    OwningWeapon = InOwningWeapon;
    OwningWeaponDataAsset = InWeaponDataAsset;

    UE_LOG(LogTemp, Log, TEXT("AS_Projectile::InitializeProjectile: %s - Instigator: %s, Weapon: %s, WeaponDA: %s"), 
         *GetNameSafe(this), *GetNameSafe(InInstigatorPawn), *GetNameSafe(InOwningWeapon), *GetNameSafe(InWeaponDataAsset));

    if (InInstigatorPawn)
    {
        SetOwner(InInstigatorPawn->GetController());
    }
    else if (InOwningWeapon) {
        SetOwner(InOwningWeapon->GetOwner());
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

    if (OtherActor == InstigatorPawn || OtherActor == OwningWeapon)
    {
        UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::OnHit: %s - Hit instigator or owning weapon. Ignoring."), *GetNameSafe(this));
        // return; // This might be too restrictive if projectiles can hit owner after a delay/bounce
    }

    K2_OnImpact(HitResult);

    if (bExplodeOnImpact)
    {
        UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::OnHit: %s - Exploding on impact."), *GetNameSafe(this));
        Detonate();
    }
    else
    {
        UE_LOG(LogTemp, Verbose, TEXT("AS_Projectile::OnHit: %s - Not exploding on impact. Destroying."), *GetNameSafe(this));
        Destroy();
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
    IgnoredActors.Add(this);
    if (InstigatorPawn) IgnoredActors.Add(InstigatorPawn);
    if (OwningWeapon) IgnoredActors.Add(OwningWeapon);

    UE_LOG(LogTemp, Log, TEXT("AS_Projectile::ApplyRadialDamage: %s - Applying radial damage. Base: %f, Min: %f, InnerR: %f, OuterR: %f, Location: %s"),
         *GetNameSafe(this), BaseDamage, MinimumDamage, DamageInnerRadius, DamageOuterRadius, *GetActorLocation().ToString());

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

    // Debug visuals
    if (GetWorld() && GetWorld()->IsNetMode(NM_ListenServer) || GetWorld()->IsNetMode(NM_Standalone)) // Only draw on server/standalone for clarity
    {
        DrawDebugSphere(
            GetWorld(),
            GetActorLocation(),
            DamageOuterRadius, // Outer radius
            32, // Segments
            FColor::Red,
            false, // Persistent lines
            5.0f,  // Lifetime
            0,     // Depth priority
            1.0f   // Thickness
        );
        DrawDebugSphere(
            GetWorld(),
            GetActorLocation(),
            DamageInnerRadius, // Inner radius (full damage)
            32,  // Segments
            FColor::Yellow,
            false, // Persistent lines
            5.0f,   // Lifetime
            0,      // Depth priority
            1.0f    // Thickness
        );
    }
}

bool AS_Projectile::Server_RequestDetonation_Validate() { return true; }
void AS_Projectile::Server_RequestDetonation_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("AS_Projectile::Server_RequestDetonation_Implementation: %s - Server received detonation request."), *GetNameSafe(this));
    Detonate();
}