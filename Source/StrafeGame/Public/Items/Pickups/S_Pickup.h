#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "S_Pickup.generated.h"

// Forward Declarations
class USphereComponent;
class UStaticMeshComponent;
class URotatingMovementComponent; // Optional for simple rotation
class AS_Character;

UCLASS(Abstract, Blueprintable, NotPlaceable) // Abstract as it's a base, NotPlaceable if typically spawned by a manager or level script
class STRAFEGAME_API AS_Pickup : public AActor
{
    GENERATED_BODY()

public:
    AS_Pickup();

    //~ Begin AActor Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override; // Alternative to component-specific overlap
    //~ End AActor Interface

    /** Collision component to detect overlap with characters. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> CollisionSphere;

    /** Static mesh component to visually represent the pickup. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> PickupMeshComponent;

    /** Optional: Adds simple rotation to the pickup mesh. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", AdvancedDisplay)
    TObjectPtr<URotatingMovementComponent> RotatingMovementComponent;

    /** Sound to play when the pickup is collected. Assigned in Blueprint. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Effects")
    TObjectPtr<USoundBase> PickupSound;

    /** Particle effect to play when the pickup is collected. Assigned in Blueprint. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Effects")
    TObjectPtr<UParticleSystem> PickupEffect;

    /** If true, the pickup actor will be destroyed after being collected. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pickup|Spawning")
    bool bDestroyOnPickup;

    /** Time in seconds before the pickup respawns after being collected. Only used if bDestroyOnPickup is false. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pickup|Spawning", meta = (EditCondition = "!bDestroyOnPickup", ClampMin = "0.1"))
    float RespawnTime;

    /**
     * Checks if the given character can pick up this item.
     * Server-side check.
     * @param Picker The character attempting to pick up the item.
     * @return True if the character can pick up this item, false otherwise.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Pickup")
    bool CanBePickedUp(AS_Character* Picker) const;
    virtual bool CanBePickedUp_Implementation(AS_Character* Picker) const;

protected:
    /** The current active state of the pickup. If false, it's hidden and non-interactive. Replicated. */
    UPROPERTY(Transient, ReplicatedUsing = OnRep_IsActive)
    bool bIsActive;

    UFUNCTION()
    virtual void OnRep_IsActive();

    /**
     * Server-authoritative function called when a character successfully overlaps and can pick up the item.
     * This function orchestrates the pickup process.
     * @param Picker The character that picked up the item.
     */
    virtual void OnPickedUpBy(AS_Character* Picker);

    /**
     * Core server-side logic for applying the pickup's effect to the character.
     * To be overridden by derived classes (WeaponPickup, AmmoPickup, PowerupPickup).
     * @param Picker The character receiving the pickup's effect.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Pickup")
    void GivePickupTo(AS_Character* Picker);
    virtual void GivePickupTo_Implementation(AS_Character* Picker);

    /** Server-side function to set the pickup's active state (visible and collidable). */
    virtual void SetPickupActiveState(bool bNewIsActive);

    /** Timer handle for respawning. */
    FTimerHandle RespawnTimerHandle;

    /** Called by the RespawnTimerHandle to make the pickup active again. */
    virtual void AttemptRespawn();

    /** Multicast RPC to play pickup effects (sound, particles) on all clients and server. */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayPickupEffects();
    virtual void Multicast_PlayPickupEffects_Implementation();

    // --- Blueprint Events for Cosmetic Overrides ---
    UFUNCTION(BlueprintImplementableEvent, Category = "Pickup|Events", meta = (DisplayName = "On Picked Up (Cosmetic)"))
    void K2_OnPickedUp(AS_Character* Picker);

    UFUNCTION(BlueprintImplementableEvent, Category = "Pickup|Events", meta = (DisplayName = "On Made Active (Respawned Cosmetic)"))
    void K2_OnRespawned();

    UFUNCTION(BlueprintImplementableEvent, Category = "Pickup|Events", meta = (DisplayName = "On Made Inactive (Picked Up Cosmetic)"))
    void K2_OnMadeInactive();
};