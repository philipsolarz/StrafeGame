// Source/StrafeGame/Public/Player/S_Character.h

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"     // For Enhanced Input
#include "GameplayAbilitySpec.h"  // For FGameplayAbilitySpecHandle
#include "S_Character.generated.h"

// Forward Declarations
class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshComponent;
class US_WeaponInventoryComponent;
class UStrafeMovementComponent; // Changed to the new movement component
class AS_PlayerState;
class UAbilitySystemComponent;
class US_AttributeSet;
class UInputMappingContext;
class UInputAction;
class UGameplayEffect;
class UGameplayAbility;
class AS_Weapon;
class US_WeaponPrimaryAbility;
class US_WeaponSecondaryAbility;
class UInputAction;

UCLASS(Blueprintable, Config = Game)
class STRAFEGAME_API AS_Character : public ACharacter
{
    GENERATED_BODY()

public:
    AS_Character(const FObjectInitializer& ObjectInitializer);

    //~ Begin AActor Interface
    virtual void Tick(float DeltaTime) override;
    virtual void PostInitializeComponents() override;
    //~ End AActor Interface

    //~ Begin APawn Interface
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_Controller() override;
    virtual void OnRep_PlayerState() override;
    //~ End APawn Interface

    /** Returns FirstPersonCameraComponent subobject **/
    FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
    /** Returns ThirdPersonCameraComponent subobject **/
    FORCEINLINE class UCameraComponent* GetThirdPersonCameraComponent() const { return ThirdPersonCameraComponent; }
    /** Returns ThirdPersonSpringArm subobject **/
    FORCEINLINE class USpringArmComponent* GetThirdPersonSpringArm() const { return ThirdPersonSpringArm; }
    /** Returns FP_Mesh subobject **/
    FORCEINLINE class USkeletalMeshComponent* GetFP_Mesh() const { return FP_Mesh; }
    /** Returns WeaponInventoryComponent subobject **/
    FORCEINLINE class US_WeaponInventoryComponent* GetWeaponInventoryComponent() const { return WeaponInventoryComponent; }

    /** Returns the StrafeMovementComponent subobject **/
    UFUNCTION(BlueprintPure, Category = "Character|Movement")
    UStrafeMovementComponent* GetStrafeMovementComponent() const;

    // Helper to get the PlayerState's ASC
    UFUNCTION(BlueprintPure, Category = "Character|Abilities")
    UAbilitySystemComponent* GetPlayerAbilitySystemComponent() const;

    // Helper to get the PlayerState's AttributeSet
    UFUNCTION(BlueprintPure, Category = "Character|Abilities")
    US_AttributeSet* GetPlayerAttributeSet() const;

    UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
    virtual AS_Weapon* GetCurrentWeapon() const;

    // Called by S_WeaponInventoryComponent when a weapon is equipped
    UFUNCTION()
    virtual void HandleWeaponEquipped(AS_Weapon* NewWeapon, AS_Weapon* OldWeapon);

    // Called when health reaches zero (detected by PlayerState's AttributeSet usually)
    UFUNCTION(BlueprintImplementableEvent, Category = "Character|Health")
    void K2_OnDeath();

    UFUNCTION(BlueprintPure, Category = "Character|View")
    bool IsFirstPersonView() const { return bIsFirstPersonView; }

    // Manages mesh visibility and weapon attachment based on view perspective
    UFUNCTION(BlueprintCallable, Category = "Character|View")
    void RefreshActiveMeshesAndWeaponAttachment();

protected:
    //~ Begin AActor Interface
    virtual void BeginPlay() override;
    //~ End AActor Interface

    // COMPONENTS
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USkeletalMeshComponent> FP_Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> ThirdPersonSpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> ThirdPersonCameraComponent;
    // End Components

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<US_WeaponInventoryComponent> WeaponInventoryComponent;

    // INPUT
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input")
    UInputMappingContext* WeaponMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    UInputAction* MoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    UInputAction* LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    UInputAction* JumpAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    UInputAction* PrimaryAbilityAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    UInputAction* SecondaryAbilityAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    UInputAction* NextWeaponAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    UInputAction* PreviousWeaponAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    UInputAction* ToggleCameraViewAction; // Added

    /** Toggles the in-game pause menu */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* TogglePauseMenuAction;

    // Input handler functions
    virtual void Input_Move(const FInputActionValue& InputActionValue);
    virtual void Input_Look(const FInputActionValue& InputActionValue);
    virtual void Input_Jump(const FInputActionValue& InputActionValue);
    virtual void Input_StopJumping(const FInputActionValue& InputActionValue);

    virtual void Input_PrimaryAbility_Pressed(const FInputActionValue& InputActionValue);
    virtual void Input_PrimaryAbility_Released(const FInputActionValue& InputActionValue);
    virtual void Input_SecondaryAbility_Pressed(const FInputActionValue& InputActionValue);
    virtual void Input_SecondaryAbility_Released(const FInputActionValue& InputActionValue);

    virtual void Input_NextWeapon(const FInputActionValue& InputActionValue);
    virtual void Input_PreviousWeapon(const FInputActionValue& InputActionValue);
    virtual void Input_ToggleCameraView(const FInputActionValue& InputActionValue); // Added

    /** Called to toggle the pause menu */
    void Input_TogglePauseMenu(const FInputActionValue& Value);

    // For weapon abilities - these store the InputID for the CURRENTLY EQUIPPED weapon's abilities
    TArray<FGameplayAbilitySpecHandle> CurrentWeaponAbilityHandles; // Server tracks granted handles
    int32 CurrentPrimaryAbilityInputID;
    int32 CurrentSecondaryAbilityInputID;

    // Initialization of GAS-related things on Character, called after ASC from PlayerState is available
    virtual void InitializeWithPlayerState();
    bool bHasInitializedWithPlayerState;


    // Health/Death
    virtual void HandleDeath(); // Called by PlayerState when health reaches zero
    friend class AS_PlayerState; // Allow PlayerState to call HandleDeath

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|View")
    bool bIsFirstPersonView;

    // Sockets for weapon attachment
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|WeaponAttachment")
    FName FirstPersonWeaponSocketName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|WeaponAttachment")
    FName ThirdPersonWeaponSocketName;


private:
    // Helper to bind to PlayerState's attribute changes.
    void BindToPlayerStateAttributes();
};