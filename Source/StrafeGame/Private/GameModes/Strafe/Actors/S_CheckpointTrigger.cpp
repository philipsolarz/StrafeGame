#include "GameModes/Strafe/Actors/S_CheckpointTrigger.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Player/S_Character.h"

AS_CheckpointTrigger::AS_CheckpointTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;

    // --- COLLISION FIX & VERIFICATION ---
    // Explicitly set the object type to WorldDynamic and ensure it overlaps Pawns.
    // This makes the interaction clear and less prone to profile misconfiguration.
    TriggerVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    TriggerVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    // --- END FIX ---

    TriggerVolume->SetCanEverAffectNavigation(false);
    TriggerVolume->SetGenerateOverlapEvents(true);

    EditorBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorBillboard"));
    if (EditorBillboard)
    {
        EditorBillboard->SetupAttachment(RootComponent);
    }

    CheckpointOrder = 0;
    TypeOfCheckpoint = ECheckpointType::Checkpoint;
}

void AS_CheckpointTrigger::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AS_CheckpointTrigger::OnTriggerOverlap);
    }
}

void AS_CheckpointTrigger::OnTriggerOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // --- DEBUG LOG ---
    // This is the first log that should fire. If you don't see this, the collision is the problem.
    UE_LOG(LogTemp, Warning, TEXT("[STRAFE DEBUG] OnTriggerOverlap fired for Checkpoint: %s. Overlapped Actor: %s"), *this->GetName(), *OtherActor->GetName());
    // --- END LOG ---

    if (HasAuthority())
    {
        AS_Character* PlayerCharacter = Cast<AS_Character>(OtherActor);
        if (PlayerCharacter)
        {
            UE_LOG(LogTemp, Warning, TEXT("[STRAFE DEBUG] Checkpoint: Player '%s' detected. Broadcasting to manager."), *PlayerCharacter->GetName());
            OnCheckpointReachedDelegate.Broadcast(this, PlayerCharacter);
        }
    }
}