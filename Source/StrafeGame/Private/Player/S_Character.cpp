#include "Player/S_Character.h"
#include "InputMappingContext.h"
#include "Player/S_PlayerState.h" // Include S_PlayerState
#include "Player/Components/S_WeaponInventoryComponent.h"
#include "Player/Components/S_CharacterMovementComponent.h" // Your custom CMC
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Abilities/Weapons/S_WeaponPrimaryAbility.h" 
#include "Abilities/Weapons/S_WeaponSecondaryAbility.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Net/UnrealNetwork.h"

#include "AbilitySystemComponent.h" 
#include "Player/Attributes/S_AttributeSet.h" 
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
    bUseControllerRotationPitch = true;
}

void AS_Character::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    if (WeaponInventoryComponent)
    {
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
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
            if (WeaponMappingContext)
            {
                Subsystem->AddMappingContext(WeaponMappingContext, 1);
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
        if (MoveAction) EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AS_Character::Input_Move);
        if (LookAction) EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AS_Character::Input_Look);
        if (JumpAction)
        {
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AS_Character::Input_Jump);
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AS_Character::Input_StopJumping);
        }
        if (PrimaryAbilityAction)
        {
            EnhancedInputComponent->BindAction(PrimaryAbilityAction, ETriggerEvent::Started, this, &AS_Character::Input_PrimaryAbility_Pressed);
            EnhancedInputComponent->BindAction(PrimaryAbilityAction, ETriggerEvent::Completed, this, &AS_Character::Input_PrimaryAbility_Released);
        }
        if (SecondaryAbilityAction)
        {
            EnhancedInputComponent->BindAction(SecondaryAbilityAction, ETriggerEvent::Started, this, &AS_Character::Input_SecondaryAbility_Pressed);
            EnhancedInputComponent->BindAction(SecondaryAbilityAction, ETriggerEvent::Completed, this, &AS_Character::Input_SecondaryAbility_Released);
        }
        if (NextWeaponAction) EnhancedInputComponent->BindAction(NextWeaponAction, ETriggerEvent::Started, this, &AS_Character::Input_NextWeapon);
        if (PreviousWeaponAction) EnhancedInputComponent->BindAction(PreviousWeaponAction, ETriggerEvent::Started, this, &AS_Character::Input_PreviousWeapon);
    }
}

UAbilitySystemComponent* AS_Character::GetPlayerAbilitySystemComponent() const
{
    AS_PlayerState* PS = GetPlayerState<AS_PlayerState>();
    return PS ? PS->GetAbilitySystemComponent() : nullptr;
}

US_AttributeSet* AS_Character::GetPlayerAttributeSet() const
{
    AS_PlayerState* PS = GetPlayerState<AS_PlayerState>();
    return PS ? PS->GetAttributeSet() : nullptr;
}


void AS_Character::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitializeWithPlayerState();
}

void AS_Character::OnRep_Controller()
{
    Super::OnRep_Controller();
}

void AS_Character::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
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
            ASC->InitAbilityActorInfo(PS, this);

            BindToPlayerStateAttributes();

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
        // Attribute change delegates are primarily handled within S_PlayerState.
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

    if (HasAuthority())
    {
        for (const FGameplayAbilitySpecHandle& Handle : CurrentWeaponAbilityHandles)
        {
            ASC->ClearAbility(Handle);
        }
        CurrentWeaponAbilityHandles.Empty();
    }

    CurrentPrimaryAbilityInputID = -1;
    CurrentSecondaryAbilityInputID = -1;

    if (NewWeapon)
    {
        const US_WeaponDataAsset* WeaponData = NewWeapon->GetWeaponData();
        if (WeaponData)
        {
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
    UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    if (ASC)
    {
        Super::Jump();
    }
}

void AS_Character::Input_StopJumping(const FInputActionValue& InputActionValue)
{
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
    K2_OnDeath();

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetSimulatePhysics(true);

    GetCharacterMovement()->DisableMovement();
    GetCharacterMovement()->StopMovementImmediately();

    if (Controller)
    {
        Controller->StopMovement();
    }

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        DisableInput(PC);
    }

    UE_LOG(LogTemp, Log, TEXT("S_Character %s: HandleDeath executed."), *GetName());
}