// Source/StrafeGame/Private/Weapons/S_Weapon.cpp
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Player/S_Character.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AS_Weapon::AS_Weapon()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true); // Movement replication is often true for weapons that can be dropped/picked up

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CurrentWeaponState = EWeaponState::Idle;
    AttachSocketName = FName("WeaponSocket"); // Default socket
    OwnerCharacter = nullptr;

    UE_LOG(LogTemp, Log, TEXT("AS_Weapon::AS_Weapon: Constructor for %s"), *GetNameSafe(this));
}

void AS_Weapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(AS_Weapon, OwnerCharacter, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(AS_Weapon, CurrentWeaponState, COND_None);
}

void AS_Weapon::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("AS_Weapon::BeginPlay: %s"), *GetNameSafe(this));

    if (WeaponData && WeaponData->WeaponMeshAsset.IsValid())
    {
        WeaponMesh->SetSkeletalMesh(WeaponData->WeaponMeshAsset.LoadSynchronous());
        UE_LOG(LogTemp, Log, TEXT("AS_Weapon::BeginPlay: %s - Mesh set from WeaponData: %s"), *GetNameSafe(this), *WeaponData->WeaponMeshAsset.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_Weapon::BeginPlay: %s - WeaponData or WeaponMeshAsset is invalid."), *GetNameSafe(this));
    }
    OnRep_WeaponState(); // Ensure initial state is visually represented
}

void AS_Weapon::SetOwnerCharacter(AS_Character* NewOwnerCharacter)
{
    UE_LOG(LogTemp, Log, TEXT("AS_Weapon::SetOwnerCharacter: %s - Attempting to set owner to %s. HasAuthority: %d"), *GetNameSafe(this), *GetNameSafe(NewOwnerCharacter), HasAuthority());
    if (HasAuthority())
    {
        OwnerCharacter = NewOwnerCharacter;
        SetOwner(NewOwnerCharacter);

        if (!OwnerCharacter && CurrentWeaponState != EWeaponState::Idle)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_Weapon::SetOwnerCharacter: %s - New owner is null, setting state to Idle."), *GetNameSafe(this));
            SetWeaponState(EWeaponState::Idle);
        }
    }
}

void AS_Weapon::OnRep_OwnerCharacter()
{
    UE_LOG(LogTemp, Log, TEXT("AS_Weapon::OnRep_OwnerCharacter: %s - Replicated OwnerCharacter: %s"), *GetNameSafe(this), *GetNameSafe(OwnerCharacter));
    if (OwnerCharacter)
    {
        SetOwner(OwnerCharacter); // Make sure AActor's owner is also set on client
        // If the weapon is supposed to be equipped and attached, ensure attachment on client
        if (CurrentWeaponState == EWeaponState::Equipped && GetAttachParentActor() != OwnerCharacter)
        {
            USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh();
            if (CharacterMesh)
            {
                FName FinalSocketName = AttachSocketName;
                if (WeaponData && WeaponData->AttachmentSocketName != NAME_None)
                {
                    FinalSocketName = WeaponData->AttachmentSocketName;
                }
                UE_LOG(LogTemp, Log, TEXT("AS_Weapon::OnRep_OwnerCharacter (Client Attach): %s - Attaching to %s at socket %s"), *GetNameSafe(this), *GetNameSafe(CharacterMesh), *FinalSocketName.ToString());
                AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FinalSocketName);
            }
        }
    }
    else // OwnerCharacter is null
    {
        if (GetAttachParentActor() != nullptr)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_Weapon::OnRep_OwnerCharacter (Client Detach): %s - Detaching from parent."), *GetNameSafe(this));
            DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        }
    }
}

void AS_Weapon::Equip(AS_Character* NewOwner)
{
    UE_LOG(LogTemp, Log, TEXT("AS_Weapon::Equip: %s - Attempting to equip to %s. HasAuthority: %d"), *GetNameSafe(this), *GetNameSafe(NewOwner), HasAuthority());
    if (HasAuthority())
    {
        if (!NewOwner)
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_Weapon::Equip: %s - NewOwner is null. Unequipping if previously owned."), *GetNameSafe(this));
            if (OwnerCharacter) { Unequip(); } // This might be redundant if SetOwnerCharacter handles it
            return;
        }
        SetOwnerCharacter(NewOwner); // This will also call SetOwner
        SetWeaponState(EWeaponState::Equipping); // Server sets state, OnRep_WeaponState handles client visuals

        USkeletalMeshComponent* CharacterMesh = NewOwner->GetMesh();
        if (CharacterMesh)
        {
            FName FinalSocketName = AttachSocketName;
            if (WeaponData && WeaponData->AttachmentSocketName != NAME_None)
            {
                FinalSocketName = WeaponData->AttachmentSocketName;
            }
            UE_LOG(LogTemp, Log, TEXT("AS_Weapon::Equip (Server): %s - Attaching to %s at socket %s."), *GetNameSafe(this), *GetNameSafe(CharacterMesh), *FinalSocketName.ToString());
            AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FinalSocketName);
            SetActorHiddenInGame(false); // Ensure it's visible after attaching
        }
        SetWeaponState(EWeaponState::Equipped); // Transition to Equipped state
        K2_OnEquipped(); // Server-side BP event
    }
}

void AS_Weapon::Unequip()
{
    UE_LOG(LogTemp, Log, TEXT("AS_Weapon::Unequip: %s. HasAuthority: %d"), *GetNameSafe(this), HasAuthority());
    if (HasAuthority())
    {
        StartUnequipEffects(); // This will set state to Unequipping
        // Actual detachment and hiding can happen after unequip anim/delay, or immediately if no anim
        SetActorHiddenInGame(true); // Hide it
        // DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); // Detach if needed
        SetWeaponState(EWeaponState::Idle); // Set to Idle
        K2_OnUnequipped(); // Server-side BP event
        // SetOwnerCharacter(nullptr); // Clear owner if it's no longer associated with a character
    }
}

void AS_Weapon::StartUnequipEffects()
{
    UE_LOG(LogTemp, Log, TEXT("AS_Weapon::StartUnequipEffects: %s. CurrentState: %s, HasAuthority: %d"), *GetNameSafe(this), *UEnum::GetValueAsString(CurrentWeaponState), HasAuthority());
    if (HasAuthority())
    {
        if (CurrentWeaponState == EWeaponState::Equipped || CurrentWeaponState == EWeaponState::Equipping)
        {
            SetWeaponState(EWeaponState::Unequipping);
            K2_OnStartUnequipEffects(); // Server-side BP event
        }
    }
}

bool AS_Weapon::IsEquipped() const
{
    return CurrentWeaponState == EWeaponState::Equipped && OwnerCharacter != nullptr;
}

void AS_Weapon::OnRep_WeaponState()
{
    UE_LOG(LogTemp, Log, TEXT("AS_Weapon::OnRep_WeaponState: %s - New State: %s"), *GetNameSafe(this), *UEnum::GetValueAsString(CurrentWeaponState));
    switch (CurrentWeaponState)
    {
    case EWeaponState::Idle:
        SetActorHiddenInGame(true);
        if (GetAttachParentActor() != nullptr)
        {
            DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        }
        K2_OnUnequipped();
        break;
    case EWeaponState::Equipping:
        SetActorHiddenInGame(false); // Become visible during equipping
        // Attachment logic handled by Equip or OnRep_OwnerCharacter if owner is set
        break;
    case EWeaponState::Equipped:
        SetActorHiddenInGame(false);
        if (OwnerCharacter && GetAttachParentActor() != OwnerCharacter) // Ensure attached if owner exists
        {
            USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh();
            if (CharacterMesh)
            {
                FName FinalSocketName = AttachSocketName;
                if (WeaponData && WeaponData->AttachmentSocketName != NAME_None)
                {
                    FinalSocketName = WeaponData->AttachmentSocketName;
                }
                UE_LOG(LogTemp, Log, TEXT("AS_Weapon::OnRep_WeaponState (Equipped - Client Attach): %s - Attaching to %s at socket %s."), *GetNameSafe(this), *GetNameSafe(CharacterMesh), *FinalSocketName.ToString());
                AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FinalSocketName);
            }
        }
        K2_OnEquipped();
        break;
    case EWeaponState::Unequipping:
        // Effects for unequipping are typically started, actual hiding/detaching might wait for anim
        K2_OnStartUnequipEffects();
        break;
    default:
        break;
    }
}

void AS_Weapon::SetWeaponState(EWeaponState NewState)
{
    if (HasAuthority())
    {
        if (CurrentWeaponState != NewState)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_Weapon::SetWeaponState (Server): %s - Changing state from %s to %s"), *GetNameSafe(this), *UEnum::GetValueAsString(CurrentWeaponState), *UEnum::GetValueAsString(NewState));
            CurrentWeaponState = NewState;
            OnRep_WeaponState(); // Call OnRep manually on server for immediate effect + to mark for replication
        }
    }
}

void AS_Weapon::ExecutePrimaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData)
{
    UE_LOG(LogTemp, Warning, TEXT("AS_Weapon::ExecutePrimaryFire_Implementation called on base class %s. Override in derived classes. FireStart: %s, FireDir: %s"), *GetNameSafe(this), *FireStartLocation.ToString(), *FireDirection.ToString());
}

void AS_Weapon::ExecuteSecondaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData)
{
    UE_LOG(LogTemp, Warning, TEXT("AS_Weapon::ExecuteSecondaryFire_Implementation called on base class %s. Override in derived classes. FireStart: %s, FireDir: %s"), *GetNameSafe(this), *FireStartLocation.ToString(), *FireDirection.ToString());
}