#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"     // For Enhanced Input
#include "GameplayAbilitySpec.h"  // For FGameplayAbilitySpecHandle
// Note: AbilitySystemInterface.h is removed as ASC is on PlayerState
#include "S_Character.generated.h"

// Forward Declarations
class UCameraComponent;
class US_WeaponInventoryComponent;
class US_CharacterMovementComponent; // Assuming you'll create this custom version
class AS_PlayerState; // Forward declare, will be crucial
class UAbilitySystemComponent; // Standard ASC, as S_Character will get it from PlayerState
class US_AttributeSet; // Your custom attribute set
class UInputMappingContext;
class UInputAction;
class UGameplayEffect;
class UGameplayAbility;
class AS_Weapon; // Your new base weapon class
class US_WeaponPrimaryAbility; // Assuming a base class for weapon abilities with InputID
class US_WeaponSecondaryAbility; // Assuming a base class for weapon abilities with InputID

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
    virtual void OnRep_Controller() override; // Useful for client-side init after controller is set
    virtual void OnRep_PlayerState() override; // Crucial for client-side GAS init when PlayerState (and its ASC) arrives
    //~ End APawn Interface

    /** Returns FirstPersonCameraComponent subobject **/
    FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
    /** Returns WeaponInventoryComponent subobject **/
    FORCEINLINE class US_WeaponInventoryComponent* GetWeaponInventoryComponent() const { return WeaponInventoryComponent; }

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

protected:
    //~ Begin AActor Interface
    virtual void BeginPlay() override;
    //~ End AActor Interface

    // COMPONENTS
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<US_WeaponInventoryComponent> WeaponInventoryComponent;

    // INPUT (using TSoftObjectPtr for deferred loading)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input")
    TSoftObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input")
    TSoftObjectPtr<UInputMappingContext> WeaponMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    TSoftObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    TSoftObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    TSoftObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    TSoftObjectPtr<UInputAction> PrimaryAbilityAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    TSoftObjectPtr<UInputAction> SecondaryAbilityAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    TSoftObjectPtr<UInputAction> NextWeaponAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Enhanced Input|Actions")
    TSoftObjectPtr<UInputAction> PreviousWeaponAction;

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

private:
    // Helper to bind to PlayerState's attribute changes.
    void BindToPlayerStateAttributes();
};