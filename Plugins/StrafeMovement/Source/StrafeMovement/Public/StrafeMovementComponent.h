#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StrafeMovementComponent.generated.h"

// Forward declaration for our custom saved move
class FSavedMove_Strafe;

/**
 * Enum for movement presets
 */
UENUM(BlueprintType)
enum class EStrafeMovementPreset : uint8
{
    ClassicQuake UMETA(DisplayName = "Classic Quake"),
    Custom UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct FStrafeAngleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Strafe Angles")
    float CurrentDeltaDegrees; // Actual angle between velocity and wishdir

    UPROPERTY(BlueprintReadOnly, Category = "Strafe Angles")
    float OptimalDeltaDegrees; // δ_opt

    UPROPERTY(BlueprintReadOnly, Category = "Strafe Angles")
    float MinDeltaDegrees;     // δ_min

    UPROPERTY(BlueprintReadOnly, Category = "Strafe Angles")
    float MaxGainDeltaDegrees; // δ_max (boundary for any speed gain)

    FStrafeAngleInfo() : CurrentDeltaDegrees(0.f), OptimalDeltaDegrees(0.f), MinDeltaDegrees(0.f), MaxGainDeltaDegrees(0.f) {}
};

/**
 * UStrafeMovementComponent
 *
 * Replicates Quake III Arena's core movement mechanics within the Unreal Engine framework.
 * Focuses on ground/air acceleration, friction, jumping, and maintaining the "feel"
 * of strafe-jumping, bunny-hopping, and step-sliding.
 */
UCLASS(Config = Game)
class STRAFEMOVEMENT_API UStrafeMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    UStrafeMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~ Begin UActorComponent Interface
    virtual void InitializeComponent() override;
    //~ End UActorComponent Interface

    //~ Begin UCharacterMovementComponent Interface
    virtual float GetMaxAcceleration() const override;
    virtual float GetMaxBrakingDeceleration() const override;
    virtual void PhysWalking(float deltaTime, int32 Iterations) override;
    virtual void PhysFalling(float deltaTime, int32 Iterations) override;
    virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
    virtual bool DoJump(bool bReplayingMoves) override;
    virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
    //~ End UCharacterMovementComponent Interface

    //~ Begin INetworkPredictionInterface Interface
    virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
    //~ End INetworkPredictionInterface Interface

    /** Applies a velocity impulse, useful for knockback or special jump pads. */
    UFUNCTION(BlueprintCallable, Category = "Strafe Movement|Impulses")
    void ApplyStrafeImpulse(const FVector& Impulse, bool bVelocityChange);

    /** Sets movement parameters based on a preset. */
    UFUNCTION(BlueprintCallable, Category = "Strafe Movement|Presets")
    void SetMovementPreset(EStrafeMovementPreset NewPreset);

    /** Get current wish speed (max speed player is trying to achieve through input) */
    UFUNCTION(BlueprintPure, Category = "Strafe Movement|Debug")
    float GetWishSpeed() const { return CurrentWishSpeed; }





    UFUNCTION(BlueprintPure, Category = "Strafe Movement|HUD|WorldSpace")
    FVector2D GetWorldVelocity2D() const;

    UFUNCTION(BlueprintPure, Category = "Strafe Movement|HUD|WorldSpace")
    FVector2D GetWorldPlayerOrientation2D() const;

    UFUNCTION(BlueprintPure, Category = "Strafe Movement|HUD|WorldSpace")
    FVector2D GetWorldWishDirection2D() const;

    // New functions for Player-Relative 2D Vectors for the HUD
    UFUNCTION(BlueprintPure, Category = "Strafe Movement|HUD|PlayerRelative")
    FVector2D GetPlayerRelativeVelocity2D() const;

    UFUNCTION(BlueprintPure, Category = "Strafe Movement|HUD|PlayerRelative")
    FVector2D GetPlayerRelativeWishDirection2D() const;

    UFUNCTION(BlueprintPure, Category = "Strafe Movement|HUD")
    float GetSpeedCapS() const { return MaxWishSpeed; }

    UFUNCTION(BlueprintPure, Category = "Strafe Movement|HUD")
    float GetEffectiveAccelerationA() const;

    UFUNCTION(BlueprintPure, Category = "Strafe Movement|HUD")
    FStrafeAngleInfo GetStrafeAngleInfo() const; // This should still calculate delta based on world vectors.

    // Helper to get the rotation from world to player's local 2D orientation (where player's forward is "up")
    FRotator GetPlayerOrientationInverseRotation() const;




protected:
    //~ Begin UCharacterMovementComponent Protected Interface
    virtual void UpdateFromCompressedFlags(uint8 Flags) override;
    //~ End UCharacterMovementComponent Protected Interface

    /** Core Quake-style acceleration logic. */
    virtual void ApplyStrafeAcceleration(const FVector& WishDirection, float WishSpeed, float AccelerationParam, float DeltaTime);

    /** Core Quake-style friction logic. */
    virtual void ApplyStrafeFriction(float DeltaTime);

    /** Called when the player lands on the ground. */
    virtual void OnLanded(const FHitResult& Hit);

    /**
     * Attempts to perform a Quake-style step-up and slide.
     * Called when an initial move in PhysWalking is blocked.
     * @param InitialBlockHit The hit result from the first blocked move.
     * @param PreFrameLocation Location of the component *before* the initial blocked move was attempted.
     * @param PreFrameVelocity Velocity of the component *before* the initial blocked move was attempted.
     * @param DeltaTime The frame's delta time.
     * @return True if the step-up and subsequent move were successful, false otherwise.
     */
    virtual bool TryStrafeStepUp(const FHitResult& InitialBlockHit, const FVector& PreFrameLocation, const FVector& PreFrameVelocity, float DeltaTime);


    UPROPERTY()
    EStrafeMovementPreset CurrentMovementPreset;

public:
    // --- Strafe Movement Parameters ---

    /** Maximum speed the player wishes to achieve via input (equivalent to Q3's g_speed or ps->speed). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe Movement|Core", Config)
    float MaxWishSpeed;

    /** Ground friction factor (equivalent to Q3's pm_friction). Applied when on ground. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe Movement|Ground", Config)
    float GroundFrictionFactor;

    /** Speed below which ground friction brings the player to a full stop (equivalent to Q3's pm_stopspeed). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe Movement|Ground", Config)
    float GroundStopSpeed;

    /** Acceleration factor when on ground (equivalent to Q3's pm_accelerate). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe Movement|Ground", Config)
    float GroundAccelerationFactor;

    /** Acceleration factor when in air (equivalent to Q3's pm_airaccelerate). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe Movement|Air", Config)
    float AirAccelerationFactor;

    /** If true, air acceleration is not limited by the MaxWishSpeed. This allows for traditional bunny hopping/strafe jumping. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe Movement|Air", Config)
    bool bAirAccelerationAllowsExceedingMaxWishSpeed;

    /** Initial vertical velocity for a jump (equivalent to Q3's JUMP_VELOCITY). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe Movement|Jumping", Config)
    float StrafeJumpImpulse;

    // --- Quake Step Logic Parameters ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe Movement|Stepping", Config)
    bool bEnableQuakeStepLogic;

    /** Height for the Quake-style step-up logic (similar to Q3's STEPSIZE). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strafe Movement|Stepping", Config, meta = (EditCondition = "bEnableQuakeStepLogic", ClampMin = "0.0"))
    float QuakeStepHeight;

    // --- Movement Presets Data (Example) ---
    UPROPERTY(EditDefaultsOnly, Category = "Strafe Movement|Presets|ClassicQuake")
    float ClassicQuake_MaxWishSpeed = 320.f;
    UPROPERTY(EditDefaultsOnly, Category = "Strafe Movement|Presets|ClassicQuake")
    float ClassicQuake_GroundFrictionFactor = 6.f;
    UPROPERTY(EditDefaultsOnly, Category = "Strafe Movement|Presets|ClassicQuake")
    float ClassicQuake_GroundStopSpeed = 100.f;
    UPROPERTY(EditDefaultsOnly, Category = "Strafe Movement|Presets|ClassicQuake")
    float ClassicQuake_GroundAccelerationFactor = 10.f;
    UPROPERTY(EditDefaultsOnly, Category = "Strafe Movement|Presets|ClassicQuake")
    float ClassicQuake_AirAccelerationFactor = 1.f;
    UPROPERTY(EditDefaultsOnly, Category = "Strafe Movement|Presets|ClassicQuake")
    bool ClassicQuake_bAirAccelerationAllowsExceedingMaxWishSpeed = true;
    UPROPERTY(EditDefaultsOnly, Category = "Strafe Movement|Presets|ClassicQuake")
    float ClassicQuake_StrafeJumpImpulse = 270.f;
    UPROPERTY(EditDefaultsOnly, Category = "Strafe Movement|Presets|ClassicQuake")
    bool ClassicQuake_bEnableQuakeStepLogic = true;
    UPROPERTY(EditDefaultsOnly, Category = "Strafe Movement|Presets|ClassicQuake")
    float ClassicQuake_QuakeStepHeight = 18.f;


protected:
    /** Internal flag to track if jump button is held for network prediction. */
    bool bStrafeJumpHeld;

    /** Current wish speed based on input, before acceleration logic modifies velocity. */
    float CurrentWishSpeed;

    /** Custom Quake-style velocity clipping. */
    FVector ClipVelocity(const FVector& InVelocity, const FVector& ImpactNormal, float Overbounce = 1.001f) const;

    /** Helper to check if character is against a wall that should block Z movement when falling */
    bool IsAgainstBlockingWall(const FVector& ImpactNormal) const;

    UPROPERTY()
    bool bJustLandedFrame;

public:
    // For FSavedMove_Strafe to access
    bool GetIsStrafeJumpHeld() const { return bStrafeJumpHeld; }

    void SetIsStrafeJumpHeld(bool InValue) { bStrafeJumpHeld = InValue; }

    bool GetIsJustLandedFrame() const { return bJustLandedFrame; }
    void SetIsJustLandedFrame(bool InValue) { bJustLandedFrame = InValue; }
};


/**
 * FSavedMove_Strafe
 *
 * Custom saved move struct for network prediction.
 * Stores any additional state required by UStrafeMovementComponent that needs
 * to be reconciled between client and server.
 */

class FSavedMove_Strafe : public FSavedMove_Character
{
public:
    typedef FSavedMove_Character Super;

    bool bSavedStrafeJumpHeld;
    bool bSavedJustLandedFrame;

    //~ Begin FSavedMove_Character Interface
    virtual void Clear() override;
    virtual uint8 GetCompressedFlags() const override;
    virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
    virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
    virtual void PrepMoveFor(ACharacter* C) override;
    //~ End FSavedMove_Character Interface


};


/**
 * FNetworkPredictionData_Client_Strafe
 *
 * Custom client prediction data.
 */
class FNetworkPredictionData_Client_Strafe : public FNetworkPredictionData_Client_Character
{
public:
    typedef FNetworkPredictionData_Client_Character Super;

    FNetworkPredictionData_Client_Strafe(const UCharacterMovementComponent& ClientMovement);

    //~ Begin FNetworkPredictionData_Client_Character Interface
    virtual FSavedMovePtr AllocateNewMove() override;
    //~ End FNetworkPredictionData_Client_Character Interface
};