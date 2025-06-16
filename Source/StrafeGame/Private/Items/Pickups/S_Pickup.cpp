#include "Items/Pickups/S_Pickup.h"
#include "Components/SphereComponent.h"
#include "Components/BillboardComponent.h" // Changed from StaticMeshComponent
#include "GameFramework/RotatingMovementComponent.h"
#include "Player/S_Character.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

AS_Pickup::AS_Pickup()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(false);

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetSphereRadius(75.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    CollisionSphere->SetGenerateOverlapEvents(true);

    PickupBillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("PickupBillboard"));
    PickupBillboardComponent->SetupAttachment(CollisionSphere);
    // Billboard does not need collision

    RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
    RotatingMovementComponent->RotationRate = FRotator(0.f, 90.f, 0.f);

    bIsActive = true;
    bDestroyOnPickup = true;
    RespawnTime = 15.0f;

    PickupSound = nullptr;
    PickupEffect = nullptr;
}

void AS_Pickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(AS_Pickup, bIsActive, COND_None);
}

void AS_Pickup::BeginPlay()
{
    Super::BeginPlay();
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
    return Picker != nullptr;
}

void AS_Pickup::OnPickedUpBy(AS_Character* Picker)
{
    if (!HasAuthority() || !Picker || !bIsActive)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AS_Pickup %s: Picked up by %s."), *GetName(), *Picker->GetName());

    GivePickupTo(Picker);

    Multicast_PlayPickupEffects();
    K2_OnPickedUp(Picker);

    if (bDestroyOnPickup)
    {
        Destroy();
    }
    else
    {
        SetPickupActiveState(false);
        GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AS_Pickup::AttemptRespawn, RespawnTime, false);
    }
}

void AS_Pickup::GivePickupTo_Implementation(AS_Character* Picker)
{
    UE_LOG(LogTemp, Warning, TEXT("AS_Pickup::GivePickupTo_Implementation called on base class for %s. Override in derived pickup class!"), *GetName());
}

void AS_Pickup::SetPickupActiveState(bool bNewIsActive)
{
    if (HasAuthority())
    {
        if (bIsActive != bNewIsActive)
        {
            bIsActive = bNewIsActive;
            OnRep_IsActive();
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
    SetActorHiddenInGame(!bIsActive);
    CollisionSphere->SetCollisionEnabled(bIsActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    PickupBillboardComponent->SetVisibility(bIsActive);

    if (bIsActive)
    {
        K2_OnRespawned();
    }
    else
    {
        K2_OnMadeInactive();
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