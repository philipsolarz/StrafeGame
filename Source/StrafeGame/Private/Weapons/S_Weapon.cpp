#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Player/S_Character.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
// Make sure GameplayEffectTypes.h is included if FGameplayEventData is used, typically via S_Weapon.h

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
    DOREPLIFETIME_CONDITION(AS_Weapon, OwnerCharacter, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(AS_Weapon, CurrentWeaponState, COND_None);
}

void AS_Weapon::BeginPlay()
{
    Super::BeginPlay();

    if (WeaponData && WeaponData->WeaponMeshAsset.IsValid()) // Check TSoftObjectPtr validity
    {
        WeaponMesh->SetSkeletalMesh(WeaponData->WeaponMeshAsset.LoadSynchronous());
    }
    // Removed deprecated WeaponMesh_DEPRECATED logic as it's not in the new S_WeaponDataAsset.h
}

void AS_Weapon::SetOwnerCharacter(AS_Character* NewOwnerCharacter)
{
    if (HasAuthority())
    {
        OwnerCharacter = NewOwnerCharacter;
        SetOwner(NewOwnerCharacter); // AActor's owner
        // SetInstigator(NewOwnerCharacter); // Set Instigator as well, if appropriate

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
        // SetInstigator(OwnerCharacter);
        if (CurrentWeaponState == EWeaponState::Equipped && GetAttachParentActor() != OwnerCharacter)
        {
            USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh(); // Assuming standard character mesh
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
        // State will be set to Equipping by the ability or inventory, then to Equipped after animation/delay
        // For now, simplified:
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
        SetWeaponState(EWeaponState::Equipped); // Transition to Equipped state
        K2_OnEquipped();
    }
}

void AS_Weapon::Unequip()
{
    if (HasAuthority())
    {
        StartUnequipEffects(); // Should visually start unequip
        SetActorHiddenInGame(true); // Hide after unequip effects would finish
        SetWeaponState(EWeaponState::Idle); // Set to idle
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
    // Client-side reactions to state changes
    switch (CurrentWeaponState)
    {
    case EWeaponState::Idle:
        SetActorHiddenInGame(true);
        K2_OnUnequipped();
        break;
    case EWeaponState::Equipping:
        SetActorHiddenInGame(false);
        // Equip animation might play, K2_OnEquipped will be called when state changes to Equipped
        break;
    case EWeaponState::Equipped:
        SetActorHiddenInGame(false);
        if (OwnerCharacter && GetAttachParentActor() != OwnerCharacter) // Ensure attachment on client
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
        // Actor hidden when state transitions to Idle after unequip animation/delay
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
            // Call OnRep manually on server for immediate effect & to mark for replication
            OnRep_WeaponState();
        }
    }
}

void AS_Weapon::ExecuteFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData, float HitscanSpread, float HitscanRange, TSubclassOf<AS_Projectile> ProjectileClass) // MODIFIED
{
    UE_LOG(LogTemp, Warning, TEXT("AS_Weapon::ExecuteFire_Implementation called on base class %s. Override in derived classes."), *GetName());
}