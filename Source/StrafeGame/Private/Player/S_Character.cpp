#include "Player/S_Character.h"
#include "Player/S_PlayerState.h" // Include S_PlayerState
#include "Player/Components/S_WeaponInventoryComponent.h"
#include "Player/Components/S_CharacterMovementComponent.h" // Your custom CMC
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Abilities/Weapons/S_WeaponPrimaryAbility.h" // Assuming this base class exists
#include "Abilities/Weapons/S_WeaponSecondaryAbility.h" // Assuming this base class exists

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Net/UnrealNetwork.h"

#include "AbilitySystemComponent.h" // Standard ASC
#include "Player/Attributes/S_AttributeSet.h" // Your AttributeSet
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"

AS_Character::AS_Character(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<US_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = true;
    bHasInitializedWithPlayerState = false;

    // Create FirstPersonCameraComponent
    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCameraComponent"));
    FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent()); // Attach to capsule
    FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Adjust as needed for FPS view
    FirstPersonCameraComponent->bUsePawnControlRotation = true; // Camera rotates with controller pitch/yaw

    WeaponInventoryComponent = CreateDefaultSubobject<US_WeaponInventoryComponent>(TEXT("WeaponInventoryComponent"));
    WeaponInventoryComponent->SetIsReplicated(true);

    CurrentPrimaryAbilityInputID = -1;
    CurrentSecondaryAbilityInputID = -1;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    bUseControllerRotationYaw = true;
    // Pitch is often handled by camera directly in FPS, but character might still need it for aim offsets.
    // If your mesh doesn't need to pitch with the controller, you can set this to false
    // and ensure the camera's bUsePawnControlRotation handles pitch.
    // For simplicity, keeping it true for now like in the original StrafeCharacter.
    bUseControllerRotationPitch = true;
}

void AS_Character::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    if (WeaponInventoryComponent)
    {
        // Ensure the delegate name in S_WeaponInventoryComponent matches
        WeaponInventoryComponent->OnWeaponEquippedDelegate.AddDynamic(this, &AS_Character::HandleWeaponEquipped);
    }
}

void AS_Character::BeginPlay()
{
    Super::BeginPlay();
    // Add input mapping context for locally controlled players
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            if (DefaultMappingContext.IsValid()) // Use IsValid() for TSoftObjectPtr before loading
            {
                Subsystem->AddMappingContext(DefaultMappingContext.LoadSynchronous(), 0);
            }
            if (WeaponMappingContext.IsValid())
            {
                Subsystem->AddMappingContext(WeaponMappingContext.LoadSynchronous(), 1);
            }
        }
    }
}

void AS_Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AS_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction.IsValid()) EnhancedInputComponent->BindAction(MoveAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &AS_Character::Input_Move);
        if (LookAction.IsValid()) EnhancedInputComponent->BindAction(LookAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &AS_Character::Input_Look);
        if (JumpAction.IsValid())
        {
            EnhancedInputComponent->BindAction(JumpAction.LoadSynchronous(), ETriggerEvent::Started, this, &AS_Character::Input_Jump); // Changed to Started for GAS
            EnhancedInputComponent->BindAction(JumpAction.LoadSynchronous(), ETriggerEvent::Completed, this, &AS_Character::Input_StopJumping);
        }
        if (PrimaryAbilityAction.IsValid())
        {
            EnhancedInputComponent->BindAction(PrimaryAbilityAction.LoadSynchronous(), ETriggerEvent::Started, this, &AS_Character::Input_PrimaryAbility_Pressed);
            EnhancedInputComponent->BindAction(PrimaryAbilityAction.LoadSynchronous(), ETriggerEvent::Completed, this, &AS_Character::Input_PrimaryAbility_Released);
        }
        if (SecondaryAbilityAction.IsValid())
        {
            EnhancedInputComponent->BindAction(SecondaryAbilityAction.LoadSynchronous(), ETriggerEvent::Started, this, &AS_Character::Input_SecondaryAbility_Pressed);
            EnhancedInputComponent->BindAction(SecondaryAbilityAction.LoadSynchronous(), ETriggerEvent::Completed, this, &AS_Character::Input_SecondaryAbility_Released);
        }
        if (NextWeaponAction.IsValid()) EnhancedInputComponent->BindAction(NextWeaponAction.LoadSynchronous(), ETriggerEvent::Started, this, &AS_Character::Input_NextWeapon);
        if (PreviousWeaponAction.IsValid()) EnhancedInputComponent->BindAction(PreviousWeaponAction.LoadSynchronous(), ETriggerEvent::Started, this, &AS_Character::Input_PreviousWeapon);
    }
}

UAbilitySystemComponent* AS_Character::GetPlayerAbilitySystemComponent() const
{
    AS_PlayerState* PS = GetPlayerState<AS_PlayerState>();
    return PS ? PS->GetAbilitySystemComponent() : nullptr; // S_PlayerState will implement IAbilitySystemInterface
}

US_AttributeSet* AS_Character::GetPlayerAttributeSet() const
{
    AS_PlayerState* PS = GetPlayerState<AS_PlayerState>();
    return PS ? PS->GetAttributeSet() : nullptr; // S_PlayerState will have a getter for its AttributeSet
}


void AS_Character::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    // Server-side initialization
    InitializeWithPlayerState();
}

void AS_Character::OnRep_Controller()
{
    Super::OnRep_Controller();
    // Client-side: Controller is replicated before PlayerState usually.
    // Input bindings can happen here if not already done in BeginPlay/SetupPlayerInputComponent.
}

void AS_Character::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    // Client-side initialization
    InitializeWithPlayerState();
}

void AS_Character::InitializeWithPlayerState()
{
    if (bHasInitializedWithPlayerState) return;

    AS_PlayerState* PS = GetPlayerState<AS_PlayerState>();
    if (PS)
    {
        UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
        if (ASC)
        {
            ASC->InitAbilityActorInfo(PS, this); // Owner is PlayerState, Avatar is Character

            // Bind to PlayerState's attribute delegates on Character for convenience
            BindToPlayerStateAttributes();

            // If the Character itself has default abilities (e.g., local, non-persistent character effects)
            // they could be granted here using PlayerState's ASC.
            // Most abilities (movement, combat) will likely be on PlayerState or granted by equipped items.

            // Initialize weapon if one is already equipped on PlayerState (e.g., after respawn)
            if (WeaponInventoryComponent)
            {
                HandleWeaponEquipped(WeaponInventoryComponent->GetCurrentWeapon(), nullptr);
            }

            bHasInitializedWithPlayerState = true;
            UE_LOG(LogTemp, Log, TEXT("S_Character %s initialized with PlayerState %s."), *GetName(), *PS->GetName());
        }
    }
}

void AS_Character::BindToPlayerStateAttributes()
{
    AS_PlayerState* PS = GetPlayerState<AS_PlayerState>();
    if (PS)
    {
        // The actual binding of attribute change delegates for things like health leading to death
        // will now primarily be handled within S_PlayerState itself, as it owns the AttributeSet.
        // S_Character can react to death via a function call from S_PlayerState or a GameplayCue.
        // For now, S_PlayerState will call AS_Character::HandleDeath().
    }
}


void AS_Character::HandleWeaponEquipped(AS_Weapon* NewWeapon, AS_Weapon* OldWeapon)
{
    UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    if (!ASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("S_Character::HandleWeaponEquipped: PlayerASC is null for %s."), *GetName());
        return;
    }

    // Server is responsible for clearing old and granting new weapon abilities
    if (HasAuthority())
    {
        for (const FGameplayAbilitySpecHandle& Handle : CurrentWeaponAbilityHandles)
        {
            ASC->ClearAbility(Handle);
        }
        CurrentWeaponAbilityHandles.Empty();
    }

    // Reset local input IDs (server and client)
    CurrentPrimaryAbilityInputID = -1;
    CurrentSecondaryAbilityInputID = -1;

    if (NewWeapon)
    {
        const US_WeaponDataAsset* WeaponData = NewWeapon->GetWeaponData();
        if (WeaponData)
        {
            // Primary Ability
            if (WeaponData->PrimaryFireAbilityClass)
            {
                if (US_WeaponPrimaryAbility* AbilityCDO = Cast<US_WeaponPrimaryAbility>(WeaponData->PrimaryFireAbilityClass->GetDefaultObject()))
                {
                    CurrentPrimaryAbilityInputID = AbilityCDO->AbilityInputID;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("S_Character::HandleWeaponEquipped: PrimaryFireAbilityClass CDO for %s is not of expected type S_WeaponPrimaryAbility or derived."), *WeaponData->GetName());
                }

                if (HasAuthority())
                {
                    FGameplayAbilitySpec Spec(WeaponData->PrimaryFireAbilityClass, 1, CurrentPrimaryAbilityInputID, NewWeapon);
                    CurrentWeaponAbilityHandles.Add(ASC->GiveAbility(Spec));
                }
            }

            // Secondary Ability
            if (WeaponData->SecondaryFireAbilityClass)
            {
                if (US_WeaponSecondaryAbility* AbilityCDO = Cast<US_WeaponSecondaryAbility>(WeaponData->SecondaryFireAbilityClass->GetDefaultObject()))
                {
                    CurrentSecondaryAbilityInputID = AbilityCDO->AbilityInputID;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("S_Character::HandleWeaponEquipped: SecondaryFireAbilityClass CDO for %s is not of expected type S_WeaponSecondaryAbility or derived."), *WeaponData->GetName());
                }

                if (HasAuthority())
                {
                    FGameplayAbilitySpec Spec(WeaponData->SecondaryFireAbilityClass, 1, CurrentSecondaryAbilityInputID, NewWeapon);
                    CurrentWeaponAbilityHandles.Add(ASC->GiveAbility(Spec));
                }
            }
        }
    }
}


AS_Weapon* AS_Character::GetCurrentWeapon() const
{
    return WeaponInventoryComponent ? WeaponInventoryComponent->GetCurrentWeapon() : nullptr;
}

// INPUT HANDLERS
void AS_Character::Input_Move(const FInputActionValue& InputActionValue)
{
    FVector2D MovementVector = InputActionValue.Get<FVector2D>();
    if (Controller)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AS_Character::Input_Look(const FInputActionValue& InputActionValue)
{
    FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();
    if (Controller)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void AS_Character::Input_Jump(const FInputActionValue& InputActionValue)
{
    // For GAS, usually you trigger an ability for Jump
    UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    if (ASC)
    {
        // Assuming Jump is linked to a generic input ID or a specific GameplayTag for the jump ability
        // FGameplayTag JumpAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Jump"));
        // ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(JumpAbilityTag));

        // Or, if you have a generic "Confirm" input ID that jump ability listens to:
        // Example: int32 GenericConfirmInputID = 0; // Define this mapping appropriately
        // ASC->AbilityLocalInputPressed(GenericConfirmInputID);

        // For now, direct call to Character's jump, assuming a Jump GA will override/enhance this
        Super::Jump();
    }
}

void AS_Character::Input_StopJumping(const FInputActionValue& InputActionValue)
{
    // If jump is an ability, send release event
    // UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    // if (ASC) {
        // ASC->AbilityLocalInputReleased(GenericConfirmInputID);
    // }
    Super::StopJumping();
}

void AS_Character::Input_PrimaryAbility_Pressed(const FInputActionValue& InputActionValue)
{
    UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    if (ASC && CurrentPrimaryAbilityInputID != INDEX_NONE)
    {
        ASC->AbilityLocalInputPressed(CurrentPrimaryAbilityInputID);
    }
}

void AS_Character::Input_PrimaryAbility_Released(const FInputActionValue& InputActionValue)
{
    UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    if (ASC && CurrentPrimaryAbilityInputID != INDEX_NONE)
    {
        ASC->AbilityLocalInputReleased(CurrentPrimaryAbilityInputID);
    }
}

void AS_Character::Input_SecondaryAbility_Pressed(const FInputActionValue& InputActionValue)
{
    UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    if (ASC && CurrentSecondaryAbilityInputID != INDEX_NONE)
    {
        ASC->AbilityLocalInputPressed(CurrentSecondaryAbilityInputID);
    }
}

void AS_Character::Input_SecondaryAbility_Released(const FInputActionValue& InputActionValue)
{
    UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    if (ASC && CurrentSecondaryAbilityInputID != INDEX_NONE)
    {
        ASC->AbilityLocalInputReleased(CurrentSecondaryAbilityInputID);
    }
}

void AS_Character::Input_NextWeapon(const FInputActionValue& InputActionValue)
{
    if (WeaponInventoryComponent) WeaponInventoryComponent->ServerRequestNextWeapon();
}

void AS_Character::Input_PreviousWeapon(const FInputActionValue& InputActionValue)
{
    if (WeaponInventoryComponent) WeaponInventoryComponent->ServerRequestPreviousWeapon();
}

void AS_Character::HandleDeath()
{
    // This function is now primarily called by S_PlayerState when it detects death.
    // Character handles the "physical" aspects of dying.
    K2_OnDeath(); // Blueprint event for cosmetic effects like playing death animation

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // Enable ragdoll
    GetMesh()->SetSimulatePhysics(true);

    GetCharacterMovement()->DisableMovement();
    GetCharacterMovement()->StopMovementImmediately();

    if (Controller)
    {
        Controller->StopMovement(); // Explicitly stop controller movement too
    }

    // Detach controller from pawn so it doesn't try to possess the ragdolling corpse
    // This is often handled by the GameMode during respawn logic.
    // For now, just disable input on the controller if it's a PlayerController.
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        DisableInput(PC);
    }

    // Set a lifespan for the corpse if desired
    // SetLifeSpan(10.0f);

    // Important: Ensure the ASC on PlayerState is NOT de-initialized here.
    // The PlayerState lives on.
    UE_LOG(LogTemp, Log, TEXT("S_Character %s: HandleDeath executed."), *GetName());
}

// Replication (if any S_Character specific properties need it beyond default Character replication)
// void AS_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
// {
//    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//    // DOREPLIFETIME(AS_Character, YourReplicatedProperty);
// }