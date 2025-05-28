#include "Items/Pickups/S_Pickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Player/S_Character.h" // For AS_Character
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h" // For playing sounds/particles
#include "Engine/World.h"           // For GetWorld()
#include "TimerManager.h"         // For FTimerManager

AS_Pickup::AS_Pickup()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true; // Pickups need to replicate their state (active/inactive)
    SetReplicateMovement(false); // Usually static, but if it moves, set this to true

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetSphereRadius(75.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // Will overlap with pawns
    CollisionSphere->SetGenerateOverlapEvents(true);

    PickupMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
    PickupMeshComponent->SetupAttachment(CollisionSphere);
    PickupMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Mesh is visual only

    RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
    RotatingMovementComponent->RotationRate = FRotator(0.f, 90.f, 0.f); // Example rotation

    bIsActive = true;
    bDestroyOnPickup = true; // Default to destroy, can be overridden by respawning pickups
    RespawnTime = 15.0f;

    PickupSound = nullptr;
    PickupEffect = nullptr;
}

void AS_Pickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(AS_Pickup, bIsActive, COND_None); // Replicate bIsActive to all
}

void AS_Pickup::BeginPlay()
{
    Super::BeginPlay();
    // Initial state update based on bIsActive (e.g., if placed in level already inactive)
    OnRep_IsActive();
}

void AS_Pickup::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (HasAuthority() && bIsActive)
    {
        AS_Character* Picker = Cast<AS_Character>(OtherActor);
        if (Picker && CanBePickedUp(Picker))
        {
            OnPickedUpBy(Picker);
        }
    }
}

bool AS_Pickup::CanBePickedUp_Implementation(AS_Character* Picker) const
{
    // Base implementation: always allow pickup if the picker is valid.
    // Derived classes will override this for specific conditions (e.g., max ammo, already has weapon).
    return Picker != nullptr;
}

void AS_Pickup::OnPickedUpBy(AS_Character* Picker)
{
    if (!HasAuthority() || !Picker || !bIsActive)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AS_Pickup %s: Picked up by %s."), *GetName(), *Picker->GetName());

    // Give the pickup effect to the character (implemented by derived classes)
    GivePickupTo(Picker);

    // Play effects
    Multicast_PlayPickupEffects();
    K2_OnPickedUp(Picker); // Server-side BP event

    if (bDestroyOnPickup)
    {
        Destroy();
    }
    else
    {
        SetPickupActiveState(false); // Hide and disable collision
        GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AS_Pickup::AttemptRespawn, RespawnTime, false);
    }
}

void AS_Pickup::GivePickupTo_Implementation(AS_Character* Picker)
{
    // Base implementation does nothing.
    // Derived classes (Weapon, Ammo, Powerup) MUST override this to provide their specific item/effect.
    UE_LOG(LogTemp, Warning, TEXT("AS_Pickup::GivePickupTo_Implementation called on base class for %s. Override in derived pickup class!"), *GetName());
}

void AS_Pickup::SetPickupActiveState(bool bNewIsActive)
{
    if (HasAuthority())
    {
        if (bIsActive != bNewIsActive)
        {
            bIsActive = bNewIsActive;
            OnRep_IsActive(); // Call OnRep manually on server for immediate effect + mark for replication
        }
    }
}

void AS_Pickup::AttemptRespawn()
{
    if (HasAuthority())
    {
        UE_LOG(LogTemp, Log, TEXT("AS_Pickup %s: Respawning."), *GetName());
        SetPickupActiveState(true);
    }
}

void AS_Pickup::OnRep_IsActive()
{
    // Update visibility and collision based on active state
    SetActorHiddenInGame(!bIsActive);
    CollisionSphere->SetCollisionEnabled(bIsActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    PickupMeshComponent->SetVisibility(bIsActive); // Also control mesh visibility directly

    if (bIsActive)
    {
        K2_OnRespawned(); // Call BP event when it becomes active (respawned)
    }
    else
    {
        K2_OnMadeInactive(); // Call BP event when it becomes inactive (picked up)
    }
}

void AS_Pickup::Multicast_PlayPickupEffects_Implementation()
{
    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
    }
    if (PickupEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PickupEffect, GetActorLocation(), GetActorRotation(), true);
    }
}