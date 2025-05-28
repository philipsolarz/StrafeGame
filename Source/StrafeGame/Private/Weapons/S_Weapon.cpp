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
    SetReplicateMovement(true);

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CurrentWeaponState = EWeaponState::Idle;
    AttachSocketName = FName("WeaponSocket");
    OwnerCharacter = nullptr;
}

void AS_Weapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(AS_Weapon, OwnerCharacter, COND_OwnerOnly); // OwnerOnly as only owner needs to know for sure, others see attachment
    DOREPLIFETIME_CONDITION(AS_Weapon, CurrentWeaponState, COND_None);
}

void AS_Weapon::BeginPlay()
{
    Super::BeginPlay();

    if (WeaponData && WeaponData->WeaponMeshAsset.IsValid())
    {
        WeaponMesh->SetSkeletalMesh(WeaponData->WeaponMeshAsset.LoadSynchronous());
    }
    else if (WeaponData && WeaponData->WeaponMesh_DEPRECATED) // Your old direct pointer property
    {
        WeaponMesh->SetSkeletalMesh(WeaponData->WeaponMesh_DEPRECATED);
    }
}

void AS_Weapon::SetOwnerCharacter(AS_Character* NewOwnerCharacter)
{
    if (HasAuthority())
    {
        OwnerCharacter = NewOwnerCharacter;
        SetOwner(NewOwnerCharacter);
        SetInstigator(NewOwnerCharacter);

        if (!OwnerCharacter && CurrentWeaponState != EWeaponState::Idle)
        {
            SetWeaponState(EWeaponState::Idle);
        }
    }
}

void AS_Weapon::OnRep_OwnerCharacter()
{
    if (OwnerCharacter)
    {
        SetOwner(OwnerCharacter);
        SetInstigator(OwnerCharacter);
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
                AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FinalSocketName);
            }
        }
    }
    else
    {
        if (GetAttachParentActor() != nullptr)
        {
            DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        }
    }
}

void AS_Weapon::Equip(AS_Character* NewOwner)
{
    if (HasAuthority())
    {
        if (!NewOwner)
        {
            if (OwnerCharacter) { Unequip(); }
            return;
        }
        SetOwnerCharacter(NewOwner);
        SetWeaponState(EWeaponState::Equipping);

        USkeletalMeshComponent* CharacterMesh = NewOwner->GetMesh();
        if (CharacterMesh)
        {
            FName FinalSocketName = AttachSocketName;
            if (WeaponData && WeaponData->AttachmentSocketName != NAME_None)
            {
                FinalSocketName = WeaponData->AttachmentSocketName;
            }
            AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FinalSocketName);
            SetActorHiddenInGame(false);
        }
        SetWeaponState(EWeaponState::Equipped);
        K2_OnEquipped();
    }
}

void AS_Weapon::Unequip()
{
    if (HasAuthority())
    {
        StartUnequipEffects(); // Should trigger visual unequip process
        SetActorHiddenInGame(true);
        SetWeaponState(EWeaponState::Idle);
        // OwnerCharacter is not nulled here; inventory component manages ownership.
        // If weapon is dropped, then OwnerCharacter would be nulled.
        K2_OnUnequipped();
    }
}

void AS_Weapon::StartUnequipEffects()
{
    if (HasAuthority())
    {
        if (CurrentWeaponState == EWeaponState::Equipped || CurrentWeaponState == EWeaponState::Equipping)
        {
            SetWeaponState(EWeaponState::Unequipping);
            K2_OnStartUnequipEffects();
        }
    }
}

bool AS_Weapon::IsEquipped() const
{
    return CurrentWeaponState == EWeaponState::Equipped && OwnerCharacter != nullptr;
}

void AS_Weapon::OnRep_WeaponState()
{
    switch (CurrentWeaponState)
    {
    case EWeaponState::Idle:
        SetActorHiddenInGame(true);
        K2_OnUnequipped();
        break;
    case EWeaponState::Equipping:
        SetActorHiddenInGame(false);
        // K2_OnEquipped might be called too early if animation isn't finished.
        // Better to have equip animation complete then set state to Equipped.
        // For simplicity, OnRep might just ensure visibility.
        break;
    case EWeaponState::Equipped:
        SetActorHiddenInGame(false);
        if (OwnerCharacter && GetAttachParentActor() != OwnerCharacter)
        {
            USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh();
            if (CharacterMesh)
            {
                FName FinalSocketName = AttachSocketName;
                if (WeaponData && WeaponData->AttachmentSocketName != NAME_None) FinalSocketName = WeaponData->AttachmentSocketName;
                AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FinalSocketName);
            }
        }
        K2_OnEquipped();
        break;
    case EWeaponState::Unequipping:
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
            CurrentWeaponState = NewState;
            OnRep_WeaponState();
        }
    }
}

// Base implementation for ExecuteFire. Derived classes will provide specific logic.
void AS_Weapon::ExecuteFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData* EventData, float HitscanSpread, float HitscanRange, TSubclassOf<AS_Projectile> ProjectileClass)
{
    // Base AS_Weapon does nothing by default.
    // AS_HitscanWeapon will implement trace logic.
    // AS_ProjectileWeapon will implement projectile spawning.
    // This function is called by the GameplayAbility.
    UE_LOG(LogTemp, Warning, TEXT("AS_Weapon::ExecuteFire_Implementation called on base class %s. Override in derived classes."), *GetName());
}