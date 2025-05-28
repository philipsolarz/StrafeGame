#include "Weapons/S_Projectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Weapons/S_Weapon.h"
#include "Player/S_Character.h" // For APawn / AS_Character casting
#include "Weapons/S_WeaponDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h" // For ApplyRadialDamageWithFalloff
#include "GameFramework/DamageType.h" // For UDamageType
#include "AbilitySystemBlueprintLibrary.h" // For GameplayCue execution
#include "AbilitySystemComponent.h"    // For UAbilitySystemComponent

AS_Projectile::AS_Projectile()
{
    PrimaryActorTick.bCanEverTick = false; // Projectiles usually don't need to tick after initial launch
    bReplicates = true;
    SetReplicateMovement(true); // Essential for projectiles

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    RootComponent = CollisionComponent;
    CollisionComponent->InitSphereRadius(10.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile")); // Use a custom "Projectile" collision profile
    CollisionComponent->bReturnMaterialOnMove = true; // For physical material based effects

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
    ProjectileMovementComponent->InitialSpeed = 3000.f;
    ProjectileMovementComponent->MaxSpeed = 3000.f;
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->bShouldBounce = false; // Default, can be overridden
    ProjectileMovementComponent->ProjectileGravityScale = 0.0f; // Default to no gravity, like rockets

    ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMeshComponent->SetupAttachment(CollisionComponent);
    ProjectileMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Defaults
    bExplodeOnImpact = true;
    MaxLifetime = 5.0f; // Projectile will self-destruct after 5 seconds by default
    BaseDamage = 50.0f;
    MinimumDamage = 10.0f;
    DamageInnerRadius = 100.0f;
    DamageOuterRadius = 300.0f;
    DamageTypeClass = UDamageType::StaticClass();

    InstigatorPawn = nullptr;
    OwningWeapon = nullptr;
    OwningWeaponDataAsset = nullptr;
}

void AS_Projectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AS_Projectile, InstigatorPawn);
    DOREPLIFETIME(AS_Projectile, OwningWeapon);
    // OwningWeaponDataAsset is set on spawn for server & client, not replicated itself.
    // bExplodeOnImpact and MaxLifetime are set at design time or on spawn, not typically replicated during flight.
}

void AS_Projectile::BeginPlay()
{
    Super::BeginPlay();

    // Bind the OnHit event only on the server, as collision handling is server-authoritative.
    if (HasAuthority())
    {
        CollisionComponent->OnComponentHit.AddDynamic(this, &AS_Projectile::OnHit);
    }

    if (MaxLifetime > 0.0f)
    {
        SetLifeSpan(MaxLifetime); // AActor's built-in lifespan mechanism
    }

    K2_OnSpawned(); // Call Blueprint event
}

void AS_Projectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (OwningWeapon && HasAuthority()) // Only server should unregister
    {
        // If AS_ProjectileWeapon exists and has a specific way to unregister:
        // if (AS_ProjectileWeapon* ProjWeapon = Cast<AS_ProjectileWeapon>(OwningWeapon))
        // {
        //     ProjWeapon->UnregisterProjectile(this);
        // }
    }
    Super::EndPlay(EndPlayReason);
}


void AS_Projectile::LifeSpanExpired()
{
    // Called when the actor's lifespan is up.
    // Default behavior is to call Destroy(). We might want to Detonate instead if applicable.
    if (HasAuthority()) // Server handles detonation logic
    {
        if (bExplodeOnImpact) // Or a specific bExplodeOnLifespanEnd flag
        {
            // UE_LOG(LogTemp, Log, TEXT("AS_Projectile %s: Lifespan expired, detonating."), *GetName());
            Detonate();
        }
        else
        {
            // UE_LOG(LogTemp, Log, TEXT("AS_Projectile %s: Lifespan expired, destroying without detonation."), *GetName());
            Destroy(); // Default lifespan behavior if not exploding
        }
    }
    // Super::LifeSpanExpired(); // Calls Destroy() by default, so if we handle it, we might not call super.
}

void AS_Projectile::InitializeProjectile(APawn* InInstigatorPawn, AS_Weapon* InOwningWeapon, const US_WeaponDataAsset* InWeaponDataAsset)
{
    InstigatorPawn = InInstigatorPawn;
    SetInstigator(InInstigatorPawn); // Set AActor's Instigator
    OwningWeapon = InOwningWeapon;
    OwningWeaponDataAsset = InWeaponDataAsset; // Store for accessing cues or other shared data

    if (InInstigatorPawn)
    {
        SetOwner(InInstigatorPawn->GetController()); // Projectile's direct owner is often the Controller for damage attribution
    }
    else if (InOwningWeapon) {
        SetOwner(InOwningWeapon->GetOwner()); // Fallback to weapon's owner
    }


    // Apply projectile speed from WeaponDataAsset if specified
    if (OwningWeaponDataAsset)
    {
        // Example: If ProjectileWeaponDataAsset has LaunchSpeed
        // if (const US_ProjectileWeaponDataAsset* ProjWepData = Cast<US_ProjectileWeaponDataAsset>(OwningWeaponDataAsset))
        // {
        //     if (ProjWepData->LaunchSpeed > 0 && ProjectileMovementComponent)
        //     {
        //         ProjectileMovementComponent->InitialSpeed = ProjWepData->LaunchSpeed;
        //         ProjectileMovementComponent->MaxSpeed = ProjWepData->LaunchSpeed;
        //     }
        //     if (ProjWepData->ProjectileLifeSpan > 0) MaxLifetime = ProjWepData->ProjectileLifeSpan; // Set before BeginPlay if possible
        // }
    }
}

void AS_Projectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector HitNormal, const FHitResult& HitResult)
{
    if (!HasAuthority()) return; // Server handles hit logic

    // Avoid self-collision immediately after spawn if needed, though ProjectileMovementComponent often handles this.
    if (OtherActor == InstigatorPawn || OtherActor == OwningWeapon)
    {
        // Could add a short grace period after spawn to ignore instigator collision if required
        // For now, assume standard collision filtering is adequate.
        // return;
    }

    // UE_LOG(LogTemp, Log, TEXT("AS_Projectile %s: Hit %s."), *GetName(), OtherActor ? *OtherActor->GetName() : TEXT("World"));
    K2_OnImpact(HitResult); // Call Blueprint event

    if (bExplodeOnImpact)
    {
        Detonate();
    }
    else
    {
        // Handle non-explosive impact (e.g., stick, bounce, destroy)
        // For now, just destroy if not exploding on impact
        Destroy();
    }
}

void AS_Projectile::Detonate()
{
    if (!HasAuthority())
    {
        Server_RequestDetonation();
        return;
    }

    // UE_LOG(LogTemp, Log, TEXT("AS_Projectile %s: Detonating at %s."), *GetName(), *GetActorLocation().ToString());

    ApplyRadialDamage(); // Server applies damage

    // Trigger GameplayCue for explosion effects (visuals, sound)
    // The GameplayCue will be replicated and play on all clients.
    FGameplayTag CueToPlay = ExplosionCueTag; // Default from this projectile
    if (OwningWeaponDataAsset && OwningWeaponDataAsset->ProjectileExplosionCueTag.IsValid()) // Check if weapon DA has a more specific one
    {
        // Example: If US_ProjectileWeaponDataAsset defined ProjectileExplosionCueTag
        // if (const US_ProjectileWeaponDataAsset* ProjWepData = Cast<US_ProjectileWeaponDataAsset>(OwningWeaponDataAsset)) {
        //    if(ProjWepData->ProjectileExplosionCueTag.IsValid()) CueToPlay = ProjWepData->ProjectileExplosionCueTag;
        // }
    }

    if (CueToPlay.IsValid() && InstigatorPawn)
    {
        UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstigatorPawn);
        if (ASC)
        {
            FGameplayCueParameters CueParams;
            CueParams.Location = GetActorLocation();
            // CueParams.Normal = ImpactNormal; // If from a hit
            CueParams.Instigator = InstigatorPawn;
            CueParams.EffectCauser = this; // Projectile caused the cue
            // CueParams.SourceAbility = OwningAbility; // If projectile was spawned by an ability and you have a ref
            ASC->ExecuteGameplayCue(CueToPlay, CueParams);
        }
    }
    else {
        // Fallback to K2 event if no cue specified (less ideal for multiplayer effects)
        K2_OnExplosionEffects(GetActorLocation());
    }

    // Notify owning weapon (if it's a projectile weapon) that this projectile is gone
    if (OwningWeapon)
    {
        if (AS_ProjectileWeapon* ProjWeapon = Cast<AS_ProjectileWeapon>(OwningWeapon))
        {
            ProjWeapon->UnregisterProjectile(this);
        }
    }

    Destroy(); // Destroy the projectile actor
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
    IgnoredActors.Add(this); // Don't damage self
    if (InstigatorPawn) IgnoredActors.Add(InstigatorPawn); // Don't damage firer by default, can be configured by damage GE
    if (OwningWeapon) IgnoredActors.Add(OwningWeapon);


    UGameplayStatics::ApplyRadialDamageWithFalloff(
        this, // WorldContextObject
        BaseDamage,
        MinimumDamage, // Damage at the edge of the radius
        GetActorLocation(),
        DamageInnerRadius,  // Full damage up to this radius
        DamageOuterRadius,  // Zero damage beyond this radius
        1.0f,               // DamageFalloff exponent (1.0 for linear)
        DamageTypeClass,
        IgnoredActors,      // Actors to ignore
        this,               // DamageCauser (the projectile)
        EventInstigatorController // Controller that instigated the damage
    );
}

bool AS_Projectile::Server_RequestDetonation_Validate() { return true; }
void AS_Projectile::Server_RequestDetonation_Implementation()
{
    Detonate();
}