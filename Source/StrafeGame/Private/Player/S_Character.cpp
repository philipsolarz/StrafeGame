// Source/StrafeGame/Private/Player/S_Character.cpp
#include "Player/S_Character.h"
#include "InputMappingContext.h"
#include "Player/S_PlayerState.h"
#include "Player/Components/S_WeaponInventoryComponent.h"
#include "Player/Components/S_CharacterMovementComponent.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Abilities/Weapons/S_WeaponPrimaryAbility.h"
#include "Abilities/Weapons/S_WeaponSecondaryAbility.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h" // Added
#include "Components/SkeletalMeshComponent.h" // Added
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
    PrimaryActorTick.bCanEverTick = true; // Enable if you need Tick, e.g., for custom camera smoothing
    bHasInitializedWithPlayerState = false;

    // First Person Camera
    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCameraComponent"));
    FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
    FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Adjust as needed
    FirstPersonCameraComponent->bUsePawnControlRotation = true;

    // First Person Mesh (Arms)
    FP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Mesh"));
    FP_Mesh->SetupAttachment(FirstPersonCameraComponent);
    FP_Mesh->SetOnlyOwnerSee(true); // Only visible to the local player
    FP_Mesh->SetCastShadow(false);  // Usually, FP arms don't cast shadows
    FP_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Typically no collision for FP arms

    // Third Person Camera Boom (SpringArm)
    ThirdPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonSpringArm"));
    ThirdPersonSpringArm->SetupAttachment(RootComponent);
    ThirdPersonSpringArm->TargetArmLength = 300.0f;    // Distance from player
    ThirdPersonSpringArm->bUsePawnControlRotation = true; // Rotate arm with controller
    ThirdPersonSpringArm->SocketOffset = FVector(0.f, 50.f, 70.f); // Offset from player's root

    // Third Person Camera
    ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCameraComponent"));
    ThirdPersonCameraComponent->SetupAttachment(ThirdPersonSpringArm, USpringArmComponent::SocketName);
    ThirdPersonCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Weapon Inventory
    WeaponInventoryComponent = CreateDefaultSubobject<US_WeaponInventoryComponent>(TEXT("WeaponInventoryComponent"));
    WeaponInventoryComponent->SetIsReplicated(true);

    // Default view state
    bIsFirstPersonView = true;

    // Default Socket Names (can be overridden in Blueprint)
    FirstPersonWeaponSocketName = FName("GripPoint_FP"); // Example name, ensure this socket exists on your FP_Mesh
    ThirdPersonWeaponSocketName = FName("WeaponSocket"); // Your existing TP socket name

    CurrentPrimaryAbilityInputID = -1;
    CurrentSecondaryAbilityInputID = -1;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    bUseControllerRotationYaw = true;
    bUseControllerRotationPitch = true;
    UE_LOG(LogTemp, Log, TEXT("AS_Character::AS_Character: Constructor for %s"), *GetNameSafe(this));
}

void AS_Character::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    UE_LOG(LogTemp, Log, TEXT("AS_Character::PostInitializeComponents: %s"), *GetNameSafe(this));
    if (WeaponInventoryComponent)
    {
        WeaponInventoryComponent->OnWeaponEquippedDelegate.AddDynamic(this, &AS_Character::HandleWeaponEquipped);
    }

    // Initial camera setup
    if (IsLocallyControlled()) // Only for local player
    {
        if (bIsFirstPersonView)
        {
            FirstPersonCameraComponent->Activate();
            ThirdPersonCameraComponent->Deactivate();
        }
        else
        {
            ThirdPersonCameraComponent->Activate();
            FirstPersonCameraComponent->Deactivate();
        }
    }
}

void AS_Character::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("AS_Character::BeginPlay: %s"), *GetNameSafe(this));
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                UE_LOG(LogTemp, Log, TEXT("AS_Character::BeginPlay: %s - Adding DefaultMappingContext"), *GetNameSafe(this));
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
            if (WeaponMappingContext)
            {
                UE_LOG(LogTemp, Log, TEXT("AS_Character::BeginPlay: %s - Adding WeaponMappingContext"), *GetNameSafe(this));
                Subsystem->AddMappingContext(WeaponMappingContext, 1); // Ensure different priority if needed
            }
        }
    }
    // RefreshActiveMeshesAndWeaponAttachment(); // Initial setup called in PossessedBy/OnRep_PlayerState after ASC init
}

void AS_Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AS_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    UE_LOG(LogTemp, Log, TEXT("AS_Character::SetupPlayerInputComponent: %s"), *GetNameSafe(this));
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
            // MODIFIED HERE: Changed ETriggerEvent::Started to ETriggerEvent::Triggered
            EnhancedInputComponent->BindAction(PrimaryAbilityAction, ETriggerEvent::Triggered, this, &AS_Character::Input_PrimaryAbility_Pressed);
            EnhancedInputComponent->BindAction(PrimaryAbilityAction, ETriggerEvent::Completed, this, &AS_Character::Input_PrimaryAbility_Released);
        }
        if (SecondaryAbilityAction)
        {
            EnhancedInputComponent->BindAction(SecondaryAbilityAction, ETriggerEvent::Triggered, this, &AS_Character::Input_SecondaryAbility_Pressed);
            EnhancedInputComponent->BindAction(SecondaryAbilityAction, ETriggerEvent::Completed, this, &AS_Character::Input_SecondaryAbility_Released);
        }
        if (NextWeaponAction) EnhancedInputComponent->BindAction(NextWeaponAction, ETriggerEvent::Started, this, &AS_Character::Input_NextWeapon);
        if (PreviousWeaponAction) EnhancedInputComponent->BindAction(PreviousWeaponAction, ETriggerEvent::Started, this, &AS_Character::Input_PreviousWeapon);

        // Added: Bind ToggleCameraViewAction
        if (ToggleCameraViewAction) EnhancedInputComponent->BindAction(ToggleCameraViewAction, ETriggerEvent::Started, this, &AS_Character::Input_ToggleCameraView);

        UE_LOG(LogTemp, Log, TEXT("AS_Character::SetupPlayerInputComponent: %s - Enhanced input bindings complete."), *GetNameSafe(this));
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
    UE_LOG(LogTemp, Log, TEXT("AS_Character::PossessedBy: %s possessed by %s. IsServer: %d"), *GetNameSafe(this), *GetNameSafe(NewController), HasAuthority());
    InitializeWithPlayerState(); // Initializes GAS and then calls RefreshActiveMeshesAndWeaponAttachment
}

void AS_Character::OnRep_Controller()
{
    Super::OnRep_Controller();
    UE_LOG(LogTemp, Log, TEXT("AS_Character::OnRep_Controller: %s new controller %s. IsClient: %d"), *GetNameSafe(this), *GetNameSafe(GetController()), !HasAuthority());
    // InitializeWithPlayerState could be called here too if PlayerState might already be set,
    // but OnRep_PlayerState is usually the more reliable point for clients.
}

void AS_Character::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    UE_LOG(LogTemp, Log, TEXT("AS_Character::OnRep_PlayerState: %s new PlayerState %s. IsClient: %d"), *GetNameSafe(this), *GetNameSafe(GetPlayerState()), !HasAuthority());
    InitializeWithPlayerState(); // Initializes GAS and then calls RefreshActiveMeshesAndWeaponAttachment
}

void AS_Character::InitializeWithPlayerState()
{
    if (bHasInitializedWithPlayerState) return;

    AS_PlayerState* PS = GetPlayerState<AS_PlayerState>();
    UE_LOG(LogTemp, Log, TEXT("AS_Character::InitializeWithPlayerState: %s - PlayerState: %s. HasAuthority: %d"), *GetNameSafe(this), *GetNameSafe(PS), HasAuthority());
    if (PS)
    {
        UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
        if (ASC)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_Character::InitializeWithPlayerState: %s - ASC found, initializing ActorInfo."), *GetNameSafe(this));
            ASC->InitAbilityActorInfo(PS, this);

            BindToPlayerStateAttributes();

            bHasInitializedWithPlayerState = true;
            UE_LOG(LogTemp, Log, TEXT("S_Character %s initialized with PlayerState %s. bHasInitializedWithPlayerState = true"), *GetName(), *PS->GetName());

            // Now that ASC is up, refresh meshes and weapon (which might grant abilities)
            RefreshActiveMeshesAndWeaponAttachment();

            // Initial weapon equip and ability granting if needed
            // This HandleWeaponEquipped will use the updated view state from RefreshActiveMeshes.
            if (WeaponInventoryComponent)
            {
                UE_LOG(LogTemp, Log, TEXT("AS_Character::InitializeWithPlayerState: %s - Initial HandleWeaponEquipped call after ASC init."), *GetNameSafe(this));
                HandleWeaponEquipped(WeaponInventoryComponent->GetCurrentWeapon(), nullptr);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_Character::InitializeWithPlayerState: %s - PlayerState %s has no AbilitySystemComponent."), *GetNameSafe(this), *GetNameSafe(PS));
        }
    }
}

void AS_Character::BindToPlayerStateAttributes()
{
    UE_LOG(LogTemp, Log, TEXT("AS_Character::BindToPlayerStateAttributes: %s - (Currently empty, delegates handled in PlayerState)"), *GetNameSafe(this));
}


void AS_Character::HandleWeaponEquipped(AS_Weapon* NewWeapon, AS_Weapon* OldWeapon)
{
    UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    UE_LOG(LogTemp, Log, TEXT("AS_Character::HandleWeaponEquipped: %s - NewWeapon: %s, OldWeapon: %s. ASC Valid: %d. HasAuthority: %d"),
        *GetNameSafe(this), *GetNameSafe(NewWeapon), *GetNameSafe(OldWeapon), ASC != nullptr, HasAuthority());

    if (!ASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_Character::HandleWeaponEquipped: PlayerASC is null for %s. Cannot manage weapon abilities."), *GetNameSafe(this));
        return;
    }

    if (HasAuthority())
    {
        UE_LOG(LogTemp, Log, TEXT("AS_Character::HandleWeaponEquipped (Server): %s - Clearing %d old weapon ability handles."), *GetNameSafe(this), CurrentWeaponAbilityHandles.Num());
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
        UE_LOG(LogTemp, Log, TEXT("AS_Character::HandleWeaponEquipped: %s - Processing new weapon %s."), *GetNameSafe(this), *NewWeapon->GetName());
        const US_WeaponDataAsset* WeaponData = NewWeapon->GetWeaponData();
        if (WeaponData)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_Character::HandleWeaponEquipped: %s - WeaponData found for %s: %s."), *GetNameSafe(this), *NewWeapon->GetName(), *WeaponData->GetName());
            if (WeaponData->PrimaryFireAbilityClass)
            {
                if (US_WeaponPrimaryAbility* AbilityCDO = Cast<US_WeaponPrimaryAbility>(WeaponData->PrimaryFireAbilityClass->GetDefaultObject()))
                {
                    CurrentPrimaryAbilityInputID = AbilityCDO->AbilityInputID;
                    UE_LOG(LogTemp, Log, TEXT("AS_Character::HandleWeaponEquipped: %s - Primary Ability InputID set to %d for %s."), *GetNameSafe(this), CurrentPrimaryAbilityInputID, *WeaponData->PrimaryFireAbilityClass->GetName());
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("AS_Character::HandleWeaponEquipped: PrimaryFireAbilityClass CDO for %s is not S_WeaponPrimaryAbility."), *WeaponData->GetName());
                }

                if (HasAuthority())
                {
                    FGameplayAbilitySpec Spec(WeaponData->PrimaryFireAbilityClass, 1, CurrentPrimaryAbilityInputID, NewWeapon);
                    FGameplayAbilitySpecHandle NewHandle = ASC->GiveAbility(Spec);
                    CurrentWeaponAbilityHandles.Add(NewHandle);
                    UE_LOG(LogTemp, Log, TEXT("AS_Character::HandleWeaponEquipped (Server): %s - Granted Primary Ability %s. Handle: %s"), *GetNameSafe(this), *WeaponData->PrimaryFireAbilityClass->GetName(), *NewHandle.ToString());
                }
            }

            if (WeaponData->SecondaryFireAbilityClass)
            {
                if (US_WeaponSecondaryAbility* AbilityCDO = Cast<US_WeaponSecondaryAbility>(WeaponData->SecondaryFireAbilityClass->GetDefaultObject()))
                {
                    CurrentSecondaryAbilityInputID = AbilityCDO->AbilityInputID;
                    UE_LOG(LogTemp, Log, TEXT("AS_Character::HandleWeaponEquipped: %s - Secondary Ability InputID set to %d for %s."), *GetNameSafe(this), CurrentSecondaryAbilityInputID, *WeaponData->SecondaryFireAbilityClass->GetName());
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("AS_Character::HandleWeaponEquipped: SecondaryFireAbilityClass CDO for %s is not S_WeaponSecondaryAbility."), *WeaponData->GetName());
                }

                if (HasAuthority())
                {
                    FGameplayAbilitySpec Spec(WeaponData->SecondaryFireAbilityClass, 1, CurrentSecondaryAbilityInputID, NewWeapon);
                    FGameplayAbilitySpecHandle NewHandle = ASC->GiveAbility(Spec);
                    CurrentWeaponAbilityHandles.Add(NewHandle);
                    UE_LOG(LogTemp, Log, TEXT("AS_Character::HandleWeaponEquipped (Server): %s - Granted Secondary Ability %s. Handle: %s"), *GetNameSafe(this), *WeaponData->SecondaryFireAbilityClass->GetName(), *NewHandle.ToString());
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_Character::HandleWeaponEquipped: %s - NewWeapon %s has no WeaponData."), *GetNameSafe(this), *NewWeapon->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AS_Character::HandleWeaponEquipped: %s - NewWeapon is null, no abilities to grant."), *GetNameSafe(this));
    }
    // After abilities are handled, refresh the weapon attachment based on current view
    RefreshActiveMeshesAndWeaponAttachment();
}


AS_Weapon* AS_Character::GetCurrentWeapon() const
{
    return WeaponInventoryComponent ? WeaponInventoryComponent->GetCurrentWeapon() : nullptr;
}

void AS_Character::RefreshActiveMeshesAndWeaponAttachment()
{
    AS_Weapon* CurrentWeapon = GetCurrentWeapon();
    USkeletalMeshComponent* TP_Mesh = GetMesh(); // This is our TP_Mesh

    if (IsLocallyControlled())
    {
        if (bIsFirstPersonView)
        {
            if (TP_Mesh) TP_Mesh->SetOwnerNoSee(true);
            if (FP_Mesh) FP_Mesh->SetVisibility(true);

            if (CurrentWeapon && CurrentWeapon->GetWeaponMeshComponent() && FP_Mesh)
            {
                CurrentWeapon->GetWeaponMeshComponent()->AttachToComponent(FP_Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FirstPersonWeaponSocketName);
                UE_LOG(LogTemp, Log, TEXT("AS_Character %s: Attached weapon %s to FP_Mesh socket %s."), *GetName(), *CurrentWeapon->GetName(), *FirstPersonWeaponSocketName.ToString());
            }
            if (FirstPersonCameraComponent) FirstPersonCameraComponent->Activate();
            if (ThirdPersonCameraComponent) ThirdPersonCameraComponent->Deactivate();
        }
        else // Locally controlled, Third-Person View
        {
            if (TP_Mesh) TP_Mesh->SetOwnerNoSee(false);
            if (FP_Mesh) FP_Mesh->SetVisibility(false);

            if (CurrentWeapon && CurrentWeapon->GetWeaponMeshComponent() && TP_Mesh)
            {
                CurrentWeapon->GetWeaponMeshComponent()->AttachToComponent(TP_Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ThirdPersonWeaponSocketName);
                UE_LOG(LogTemp, Log, TEXT("AS_Character %s: Attached weapon %s to TP_Mesh socket %s (Local TP View)."), *GetName(), *CurrentWeapon->GetName(), *ThirdPersonWeaponSocketName.ToString());
            }
            if (ThirdPersonCameraComponent) ThirdPersonCameraComponent->Activate();
            if (FirstPersonCameraComponent) FirstPersonCameraComponent->Deactivate();
        }
    }
    else // Simulated Proxy (other players)
    {
        if (TP_Mesh) TP_Mesh->SetOwnerNoSee(false); // Should be visible
        if (FP_Mesh) FP_Mesh->SetVisibility(false);  // Should not be visible

        // For simulated proxies, the weapon attachment is typically handled by AS_Weapon's OnRep_OwnerCharacter or Equip logic on server,
        // attaching to the character's main mesh (TP_Mesh).
        // We can ensure it here too if needed, but standard replication should handle it.
        if (CurrentWeapon && CurrentWeapon->GetWeaponMeshComponent() && TP_Mesh)
        {
            // Check if it's already attached to the correct component, to avoid redundant calls if OnRep_OwnerCharacter handles it.
            if (CurrentWeapon->GetWeaponMeshComponent()->GetAttachParent() != TP_Mesh)
            {
                CurrentWeapon->GetWeaponMeshComponent()->AttachToComponent(TP_Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ThirdPersonWeaponSocketName);
                UE_LOG(LogTemp, Log, TEXT("AS_Character %s (Proxy): Attached weapon %s to TP_Mesh socket %s."), *GetName(), *CurrentWeapon->GetName(), *ThirdPersonWeaponSocketName.ToString());
            }
        }
    }
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
    UE_LOG(LogTemp, Log, TEXT("AS_Character::Input_Jump: %s"), *GetNameSafe(this));
    UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    if (ASC)
    {
        // Example: Try activate jump ability by tag
        // FGameplayTag JumpTag = FGameplayTag::RequestGameplayTag(FName("Ability.Action.Jump"));
        // ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(JumpTag));
        Super::Jump(); // For now, standard jump
    }
    else {
        Super::Jump();
    }
}

void AS_Character::Input_StopJumping(const FInputActionValue& InputActionValue)
{
    UE_LOG(LogTemp, Log, TEXT("AS_Character::Input_StopJumping: %s"), *GetNameSafe(this));
    Super::StopJumping();
}

void AS_Character::Input_PrimaryAbility_Pressed(const FInputActionValue& InputActionValue)
{
    UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    UE_LOG(LogTemp, Log, TEXT("AS_Character::Input_PrimaryAbility_Pressed: %s - InputID: %d. ASC Valid: %d"), *GetNameSafe(this), CurrentPrimaryAbilityInputID, ASC != nullptr);
    if (ASC && CurrentPrimaryAbilityInputID != INDEX_NONE)
    {
        ASC->AbilityLocalInputPressed(CurrentPrimaryAbilityInputID);
    }
}

void AS_Character::Input_PrimaryAbility_Released(const FInputActionValue& InputActionValue)
{
    UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    UE_LOG(LogTemp, Log, TEXT("AS_Character::Input_PrimaryAbility_Released: %s - InputID: %d. ASC Valid: %d"), *GetNameSafe(this), CurrentPrimaryAbilityInputID, ASC != nullptr);
    if (ASC && CurrentPrimaryAbilityInputID != INDEX_NONE)
    {
        ASC->AbilityLocalInputReleased(CurrentPrimaryAbilityInputID);
    }
}

void AS_Character::Input_SecondaryAbility_Pressed(const FInputActionValue& InputActionValue)
{
    UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    UE_LOG(LogTemp, Log, TEXT("AS_Character::Input_SecondaryAbility_Pressed: %s - InputID: %d. ASC Valid: %d"), *GetNameSafe(this), CurrentSecondaryAbilityInputID, ASC != nullptr);
    if (ASC && CurrentSecondaryAbilityInputID != INDEX_NONE)
    {
        ASC->AbilityLocalInputPressed(CurrentSecondaryAbilityInputID);
    }
}

void AS_Character::Input_SecondaryAbility_Released(const FInputActionValue& InputActionValue)
{
    UAbilitySystemComponent* ASC = GetPlayerAbilitySystemComponent();
    UE_LOG(LogTemp, Log, TEXT("AS_Character::Input_SecondaryAbility_Released: %s - InputID: %d. ASC Valid: %d"), *GetNameSafe(this), CurrentSecondaryAbilityInputID, ASC != nullptr);
    if (ASC && CurrentSecondaryAbilityInputID != INDEX_NONE)
    {
        ASC->AbilityLocalInputReleased(CurrentSecondaryAbilityInputID);
    }
}

void AS_Character::Input_NextWeapon(const FInputActionValue& InputActionValue)
{
    UE_LOG(LogTemp, Log, TEXT("AS_Character::Input_NextWeapon: %s. Inventory Valid: %d"), *GetNameSafe(this), WeaponInventoryComponent != nullptr);
    if (WeaponInventoryComponent) WeaponInventoryComponent->ServerRequestNextWeapon();
}

void AS_Character::Input_PreviousWeapon(const FInputActionValue& InputActionValue)
{
    UE_LOG(LogTemp, Log, TEXT("AS_Character::Input_PreviousWeapon: %s. Inventory Valid: %d"), *GetNameSafe(this), WeaponInventoryComponent != nullptr);
    if (WeaponInventoryComponent) WeaponInventoryComponent->ServerRequestPreviousWeapon();
}

void AS_Character::Input_ToggleCameraView(const FInputActionValue& InputActionValue)
{
    UE_LOG(LogTemp, Log, TEXT("AS_Character::Input_ToggleCameraView: %s. Current FP: %d"), *GetNameSafe(this), bIsFirstPersonView);
    if (IsLocallyControlled()) // Only local player can toggle their own camera
    {
        bIsFirstPersonView = !bIsFirstPersonView;
        RefreshActiveMeshesAndWeaponAttachment(); // This will also activate/deactivate cameras
        UE_LOG(LogTemp, Log, TEXT("AS_Character::Input_ToggleCameraView: Toggled to FP: %d"), bIsFirstPersonView);
    }
}

void AS_Character::HandleDeath()
{
    UE_LOG(LogTemp, Log, TEXT("AS_Character::HandleDeath: %s executing death logic."), *GetNameSafe(this));
    K2_OnDeath();

    USkeletalMeshComponent* TP_Mesh = GetMesh();

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (TP_Mesh)
    {
        TP_Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        TP_Mesh->SetSimulatePhysics(true);
    }
    if (FP_Mesh) // Ensure FP mesh is also handled
    {
        FP_Mesh->SetVisibility(false);
        FP_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }


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
    UE_LOG(LogTemp, Log, TEXT("S_Character %s: HandleDeath executed. Collision and input disabled."), *GetName());
}