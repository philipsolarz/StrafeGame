#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "S_CheckpointTrigger.generated.h"

class UBoxComponent;
class UBillboardComponent;
class AS_Character;

// Using ECheckpointType from your old ACheckpointTrigger.h
UENUM(BlueprintType)
enum class ECheckpointType : uint8
{
    Start UMETA(DisplayName = "Start Line"),
    Checkpoint UMETA(DisplayName = "Checkpoint"),
    Finish UMETA(DisplayName = "Finish Line")
};

// Delegate broadcast when a character overlaps this checkpoint
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSCheckpointReachedDelegate, AS_CheckpointTrigger*, Checkpoint, AS_Character*, PlayerCharacter);

UCLASS(Blueprintable, ClassGroup = (Custom))
class STRAFEGAME_API AS_CheckpointTrigger : public AActor
{
    GENERATED_BODY()

public:
    AS_CheckpointTrigger();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> TriggerVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", AdvancedDisplay)
    TObjectPtr<UBillboardComponent> EditorBillboard;

    /** Order of this checkpoint in the race sequence. Start is typically 0. Must be unique among checkpoints. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint Settings", meta = (ExposeOnSpawn = "true"))
    int32 CheckpointOrder;

    /** Type of this checkpoint (Start, Finish, or intermediate). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint Settings", meta = (ExposeOnSpawn = "true"))
    ECheckpointType TypeOfCheckpoint;

    /** Delegate broadcast (on server) when a player character enters the trigger volume. */
    UPROPERTY(BlueprintAssignable, Category = "Checkpoint Events")
    FOnSCheckpointReachedDelegate OnCheckpointReachedDelegate;

    UFUNCTION(BlueprintPure, Category = "Checkpoint Settings")
    int32 GetCheckpointOrder() const { return CheckpointOrder; }

    UFUNCTION(BlueprintPure, Category = "Checkpoint Settings")
    ECheckpointType GetCheckpointType() const { return TypeOfCheckpoint; }

protected:
    //~ Begin AActor Interface
    virtual void BeginPlay() override;
    //~ End AActor Interface

    UFUNCTION() // Must be UFUNCTION to bind to overlap events
        virtual void OnTriggerOverlap(
            UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex,
            bool bFromSweep,
            const FHitResult& SweepResult
        );
};