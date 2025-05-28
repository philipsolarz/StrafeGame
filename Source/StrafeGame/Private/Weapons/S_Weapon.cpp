// Source/StrafeGame/Private/Weapons/S_Weapon.cpp
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

    if (WeaponData && WeaponData->WeaponMeshAsset.IsValid()) // Check TSoftObjectPtr validity
    {
        WeaponMesh->SetSkeletalMesh(WeaponData->WeaponMeshAsset.LoadSynchronous());
        UE_LOG(LogTemp, Log, TEXT("AS_Weapon::BeginPlay: %s - Mesh set from WeaponData: %s"), *GetNameSafe(this), *WeaponData->WeaponMeshAsset.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_Weapon::BeginPlay: %s - WeaponData or WeaponMeshAsset is invalid."), *GetNameSafe(this));
    }
}

void AS_Weapon::SetOwnerCharacter(AS_Character* NewOwnerCharacter)
{
    UE_LOG(LogTemp, Log, TEXT("AS_Weapon::SetOwnerCharacter: %s - Attempting to set owner to %s"), *GetNameSafe(this), *GetNameSafe(NewOwnerCharacter));
    if (HasAuthority())
    {
        OwnerCharacter = NewOwnerCharacter;
        SetOwner(NewOwnerCharacter); // AActor's owner

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
        SetOwner(OwnerCharacter);
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
                UE_LOG(LogTemp, Log, TEXT("AS_Weapon::OnRep_OwnerCharacter: %s - Attaching to %s at socket %s"), *GetNameSafe(this), *GetNameSafe(CharacterMesh), *FinalSocketName.ToString());
                AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FinalSocketName);
            }
        }
    }
    else
    {
        if (GetAttachParentActor() != nullptr)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_Weapon::OnRep_OwnerCharacter: %s - Detaching from parent."), *GetNameSafe(this));
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
            UE_LOG(LogTemp, Log, TEXT("AS_Weapon::Equip: %s - Attaching to %s at socket %s."), *GetNameSafe(this), *GetNameSafe(CharacterMesh), *FinalSocketName.ToString());
            AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FinalSocketName);
            SetActorHiddenInGame(false);
        }
        SetWeaponState(EWeaponState::Equipped);
        K2_OnEquipped();
    }
}

void AS_Weapon::Unequip()
{
    UE_LOG(LogTemp, Log, TEXT("AS_Weapon::Unequip: %s. HasAuthority: %d"), *GetNameSafe(this), HasAuthority());
    if (HasAuthority())
    {
        StartUnequipEffects();
        SetActorHiddenInGame(true);
        SetWeaponState(EWeaponState::Idle);
        K2_OnUnequipped();
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
    UE_LOG(LogTemp, Log, TEXT("AS_Weapon::OnRep_WeaponState: %s - New State: %s"), *GetNameSafe(this), *UEnum::GetValueAsString(CurrentWeaponState));
    switch (CurrentWeaponState)
    {
    case EWeaponState::Idle:
        SetActorHiddenInGame(true);
        K2_OnUnequipped();
        break;
    case EWeaponState::Equipping:
        SetActorHiddenInGame(false);
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
                UE_LOG(LogTemp, Log, TEXT("AS_Weapon::OnRep_WeaponState (Equipped): %s - Attaching to %s at socket %s on client."), *GetNameSafe(this), *GetNameSafe(CharacterMesh), *FinalSocketName.ToString());
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
            UE_LOG(LogTemp, Log, TEXT("AS_Weapon::SetWeaponState: %s - Changing state from %s to %s"), *GetNameSafe(this), *UEnum::GetValueAsString(CurrentWeaponState), *UEnum::GetValueAsString(NewState));
            CurrentWeaponState = NewState;
            OnRep_WeaponState();
        }
    }
}

void AS_Weapon::ExecuteFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData, float HitscanSpread, float HitscanRange, TSubclassOf<AS_Projectile> ProjectileClass)
{
    UE_LOG(LogTemp, Warning, TEXT("AS_Weapon::ExecuteFire_Implementation called on base class %s. Override in derived classes. FireStart: %s, FireDir: %s"), *GetNameSafe(this), *FireStartLocation.ToString(), *FireDirection.ToString());
}