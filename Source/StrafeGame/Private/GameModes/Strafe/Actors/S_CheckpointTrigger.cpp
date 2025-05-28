#include "GameModes/Strafe/Actors/S_CheckpointTrigger.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Player/S_Character.h" // For AS_Character
// #include "GameModes/Strafe/S_StrafeManager.h" // Not directly needed, communicates via delegate

AS_CheckpointTrigger::AS_CheckpointTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;
    TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // Ensure it overlaps with Pawns
    TriggerVolume->SetCanEverAffectNavigation(false);
    TriggerVolume->SetGenerateOverlapEvents(true);

    EditorBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorBillboard"));
    if (EditorBillboard)
    {
        EditorBillboard->SetupAttachment(RootComponent);
        // Assign a default texture to billboard if desired, or do in Blueprint
    }

    CheckpointOrder = 0;
    TypeOfCheckpoint = ECheckpointType::Checkpoint;

    // Checkpoints are part of the level, server will detect overlaps.
    // No need for bReplicates = true unless clients need to know about their state directly,
    // but StrafeManager will replicate the sorted list of checkpoints.
    // For simplicity, let's assume they are static level actors.
    // If they can be spawned dynamically AND clients need to know about them, set bReplicates = true.
}

void AS_CheckpointTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap event only on the server
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
    if (HasAuthority()) // Server processes overlaps
    {
        AS_Character* PlayerCharacter = Cast<AS_Character>(OtherActor);
        if (PlayerCharacter)
        {
            // UE_LOG(LogTemp, Log, TEXT("AS_CheckpointTrigger '%s' (Order: %d, Type: %s) overlapped by %s."),
            //     *GetName(), CheckpointOrder, *UEnum::GetValueAsString(TypeOfCheckpoint), *PlayerCharacter->GetName());

            // Broadcast to the StrafeManager (or any other listener)
            OnCheckpointReachedDelegate.Broadcast(this, PlayerCharacter);
        }
    }
}