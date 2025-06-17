#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "S_Pickup.generated.h"

// Forward Declarations
class USphereComponent;
class UBillboardComponent; // Changed from UStaticMeshComponent
class URotatingMovementComponent;
class AS_Character;

UCLASS(Abstract, Blueprintable)
class STRAFEGAME_API AS_Pickup : public AActor
{
    GENERATED_BODY()

public:
    AS_Pickup();

    //~ Begin AActor Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
    //~ End AActor Interface

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> CollisionSphere;

    /** Billboard component to visually represent the pickup. Replaces the mesh. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBillboardComponent> PickupBillboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", AdvancedDisplay)
    TObjectPtr<URotatingMovementComponent> RotatingMovementComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Effects")
    TObjectPtr<USoundBase> PickupSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Effects")
    TObjectPtr<UParticleSystem> PickupEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pickup|Spawning")
    bool bDestroyOnPickup;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pickup|Spawning", meta = (EditCondition = "!bDestroyOnPickup", ClampMin = "0.1"))
    float RespawnTime;

    UFUNCTION(BlueprintNativeEvent, Category = "Pickup")
    bool CanBePickedUp(AS_Character* Picker) const;
    virtual bool CanBePickedUp_Implementation(AS_Character* Picker) const;

protected:
    UPROPERTY(Transient, ReplicatedUsing = OnRep_IsActive)
    bool bIsActive;

    UFUNCTION()
    virtual void OnRep_IsActive();

    virtual void OnPickedUpBy(AS_Character* Picker);

    UFUNCTION(BlueprintNativeEvent, Category = "Pickup")
    void GivePickupTo(AS_Character* Picker);
    virtual void GivePickupTo_Implementation(AS_Character* Picker);

    virtual void SetPickupActiveState(bool bNewIsActive);

    FTimerHandle RespawnTimerHandle;

    virtual void AttemptRespawn();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayPickupEffects();
    virtual void Multicast_PlayPickupEffects_Implementation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Pickup|Events", meta = (DisplayName = "On Picked Up (Cosmetic)"))
    void K2_OnPickedUp(AS_Character* Picker);

    UFUNCTION(BlueprintImplementableEvent, Category = "Pickup|Events", meta = (DisplayName = "On Respawned (Cosmetic)"))
    void K2_OnRespawned();

    UFUNCTION(BlueprintImplementableEvent, Category = "Pickup|Events", meta = (DisplayName = "On Made Inactive (Picked Up Cosmetic)"))
    void K2_OnMadeInactive();
};