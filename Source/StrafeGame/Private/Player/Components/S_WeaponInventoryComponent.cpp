// Source/StrafeGame/Private/Player/Components/S_WeaponInventoryComponent.cpp
#include "Player/Components/S_WeaponInventoryComponent.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Player/S_Character.h"
#include "Player/S_PlayerState.h"      // To get PlayerState for ASC
#include "AbilitySystemComponent.h"   // For applying GameplayEffects
#include "GameplayEffect.h"           // For UGameplayEffect
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"      // For UActorChannel for weapon replication
#include "TimerManager.h"           // For FTimerManager
#include "GameFramework/Character.h"  // For ACharacter casting (though S_Character is preferred)

US_WeaponInventoryComponent::US_WeaponInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    CurrentWeapon = nullptr;
    PendingWeapon = nullptr;
    OwningCharacter = nullptr;
    OwnerAbilitySystemComponent = nullptr;
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::US_WeaponInventoryComponent: Constructor for component on %s"), *GetNameSafe(GetOwner()));
}

void US_WeaponInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(US_WeaponInventoryComponent, WeaponInventory);
    DOREPLIFETIME(US_WeaponInventoryComponent, CurrentWeapon);
}

void US_WeaponInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::BeginPlay: Component for %s"), *GetNameSafe(GetOwner()));

    CacheOwnerReferences();

    if (GetOwnerRole() == ROLE_Authority && OwningCharacter)
    {
        UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::BeginPlay: Server - Granting starting weapons for %s."), *OwningCharacter->GetName());
        for (TSubclassOf<AS_Weapon> WeaponClass : StartingWeaponClasses)
        {
            if (WeaponClass)
            {
                UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::BeginPlay: Adding starting weapon %s."), *WeaponClass->GetName());
                ServerAddWeapon(WeaponClass);
            }
        }

        if (StartingWeaponClasses.Num() > 0 && WeaponInventory.Num() > 0 && WeaponInventory[0]->IsA(StartingWeaponClasses[0]))
        {
            UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::BeginPlay: Equipping first starting weapon %s."), *StartingWeaponClasses[0]->GetName());
            ServerEquipWeaponByClass(StartingWeaponClasses[0]);
        }
        else if (WeaponInventory.Num() > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::BeginPlay: Equipping first available weapon %s."), *WeaponInventory[0]->GetClass()->GetName());
            ServerEquipWeaponByClass(WeaponInventory[0]->GetClass());
        }
    }
}

void US_WeaponInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetOwnerRole() == ROLE_Authority)
    {
        for (AS_Weapon* Weapon : WeaponInventory)
        {
            if (Weapon)
            {
                UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::EndPlay: Destroying weapon %s."), *Weapon->GetName());
                Weapon->Destroy();
            }
        }
        WeaponInventory.Empty();
        CurrentWeapon = nullptr;
        PendingWeapon = nullptr;
    }
    Super::EndPlay(EndPlayReason);
}


bool US_WeaponInventoryComponent::CacheOwnerReferences()
{
    if (!OwningCharacter)
    {
        OwningCharacter = Cast<AS_Character>(GetOwner());
    }
    if (OwningCharacter && !OwnerAbilitySystemComponent)
    {
        AS_PlayerState* PS = OwningCharacter->GetPlayerState<AS_PlayerState>();
        if (PS)
        {
            OwnerAbilitySystemComponent = PS->GetAbilitySystemComponent();
        }
    }
    if (!OwningCharacter) UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent::CacheOwnerReferences: OwningCharacter is NULL for %s"), *GetNameSafe(GetOwner()));
        if (!OwnerAbilitySystemComponent && OwningCharacter) UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent::CacheOwnerReferences: OwnerAbilitySystemComponent is NULL for %s"), *OwningCharacter->GetName());
            return OwningCharacter && OwnerAbilitySystemComponent;
}

AS_Weapon* US_WeaponInventoryComponent::SpawnWeaponActor(TSubclassOf<AS_Weapon> WeaponClass)
{
    if (!WeaponClass || GetOwnerRole() != ROLE_Authority || !GetWorld() || !OwningCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent::SpawnWeaponActor: Preconditions not met for spawning %s. WeaponClass Valid: %d, IsServer: %d, World Valid: %d, OwningChar Valid: %d"), 
             *GetNameSafe(WeaponClass), WeaponClass != nullptr, GetOwnerRole() == ROLE_Authority, GetWorld() != nullptr, OwningCharacter != nullptr);
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwningCharacter;
    SpawnParams.Instigator = OwningCharacter;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AS_Weapon* NewWeapon = GetWorld()->SpawnActor<AS_Weapon>(WeaponClass, OwningCharacter->GetActorLocation(), OwningCharacter->GetActorRotation(), SpawnParams);
    if (NewWeapon)
    {
        UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::SpawnWeaponActor: Successfully spawned %s for %s."), *NewWeapon->GetName(), *OwningCharacter->GetName());
        NewWeapon->SetActorHiddenInGame(true);
        NewWeapon->SetOwnerCharacter(OwningCharacter);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("US_WeaponInventoryComponent::SpawnWeaponActor: Failed to spawn weapon of class %s for %s."), *WeaponClass->GetName(), *OwningCharacter->GetName());
    }
    return NewWeapon;
}

void US_WeaponInventoryComponent::ApplyInitialAmmoForWeapon(AS_Weapon* WeaponToGrantAmmo)
{
    if (GetOwnerRole() != ROLE_Authority || !WeaponToGrantAmmo || !CacheOwnerReferences())
    {
        UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent::ApplyInitialAmmoForWeapon: Preconditions not met for %s."), *GetNameSafe(WeaponToGrantAmmo));
        return;
    }

    const US_WeaponDataAsset* WeaponData = WeaponToGrantAmmo->GetWeaponData();
    if (OwnerAbilitySystemComponent && WeaponData && WeaponData->InitialAmmoEffect)
    {
        FGameplayEffectContextHandle EffectContext = OwnerAbilitySystemComponent->MakeEffectContext();
        EffectContext.AddSourceObject(WeaponToGrantAmmo);

        FGameplayEffectSpecHandle SpecHandle = OwnerAbilitySystemComponent->MakeOutgoingSpec(WeaponData->InitialAmmoEffect, 1, EffectContext);
        if (SpecHandle.IsValid())
        {
            OwnerAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ApplyInitialAmmoForWeapon: Applied InitialAmmoEffect %s for weapon %s on %s"),
                 *WeaponData->InitialAmmoEffect->GetName(), *WeaponToGrantAmmo->GetName(), *OwningCharacter->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent::ApplyInitialAmmoForWeapon: Failed to create spec for InitialAmmoEffect %s on %s"), *WeaponData->InitialAmmoEffect->GetName(), *OwningCharacter->GetName());
        }
    }
    else
    {
        if (!WeaponData)
        {
            UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent::ApplyInitialAmmoForWeapon: Weapon %s has no WeaponData."), *WeaponToGrantAmmo->GetName());
        }
        else if (!WeaponData->InitialAmmoEffect) {
            UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent::ApplyInitialAmmoForWeapon: WeaponData for %s has no InitialAmmoEffect set."), *WeaponToGrantAmmo->GetName());
        }
        else if (!OwnerAbilitySystemComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent::ApplyInitialAmmoForWeapon: OwnerASC is null, cannot apply initial ammo for %s."), *WeaponToGrantAmmo->GetName());
        }
    }
}


bool US_WeaponInventoryComponent::ServerAddWeapon(TSubclassOf<AS_Weapon> WeaponClass)
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerAddWeapon: %s attempting to add %s. IsServer: %d"), *GetNameSafe(GetOwner()), *GetNameSafe(WeaponClass), GetOwnerRole() == ROLE_Authority);
    if (GetOwnerRole() != ROLE_Authority || !WeaponClass || !CacheOwnerReferences())
    {
        return false;
    }

    for (AS_Weapon* ExistingWeapon : WeaponInventory)
    {
        if (ExistingWeapon && ExistingWeapon->IsA(WeaponClass))
        {
            UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerAddWeapon: Player %s already has %s. Applying initial ammo."), *OwningCharacter->GetName(), *WeaponClass->GetName());
            ApplyInitialAmmoForWeapon(ExistingWeapon);
            OnWeaponAddedDelegate.Broadcast(WeaponClass);
            return true;
        }
    }

    AS_Weapon* NewWeapon = SpawnWeaponActor(WeaponClass);
    if (NewWeapon)
    {
        WeaponInventory.Add(NewWeapon);
        OnRep_WeaponInventory();

        ApplyInitialAmmoForWeapon(NewWeapon);

        OnWeaponAddedDelegate.Broadcast(WeaponClass);
        UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerAddWeapon: Added weapon %s to %s's inventory. Total weapons: %d"), *NewWeapon->GetName(), *OwningCharacter->GetName(), WeaponInventory.Num());
        return true;
    }
    UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent::ServerAddWeapon: Failed to spawn new weapon %s for %s."), *WeaponClass->GetName(), *OwningCharacter->GetName());
    return false;
}

void US_WeaponInventoryComponent::ServerEquipWeaponByClass(TSubclassOf<AS_Weapon> WeaponClass)
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerEquipWeaponByClass: %s attempting to equip %s. IsServer: %d"), *GetNameSafe(GetOwner()), *GetNameSafe(WeaponClass), GetOwnerRole() == ROLE_Authority);
    if (GetOwnerRole() != ROLE_Authority || !OwningCharacter) return;

    if (!WeaponClass)
    {
        if (CurrentWeapon)
        {
            UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerEquipWeaponByClass: Requested unequip for %s."), *OwningCharacter->GetName());
            PendingWeapon = nullptr;
            GetWorld()->GetTimerManager().ClearTimer(WeaponSwitchTimerHandle);

            AS_Weapon* OldWeapon = CurrentWeapon;
            CurrentWeapon->Unequip();
            CurrentWeapon = nullptr;
            OnRep_CurrentWeapon();
            OnWeaponEquippedDelegate.Broadcast(nullptr, OldWeapon);
            UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerEquipWeaponByClass: Unequipped weapon on %s."), *OwningCharacter->GetName());
        }
        return;
    }

    AS_Weapon* WeaponToEquip = nullptr;
    for (AS_Weapon* WeaponInInv : WeaponInventory)
    {
        if (WeaponInInv && WeaponInInv->IsA(WeaponClass))
        {
            WeaponToEquip = WeaponInInv;
            break;
        }
    }

    if (!WeaponToEquip)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent::ServerEquipWeaponByClass: %s trying to equip %s, but not found in inventory."), *OwningCharacter->GetName(), *WeaponClass->GetName());
        return;
    }

    if (WeaponToEquip == CurrentWeapon && WeaponToEquip->IsEquipped())
    {
        UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerEquipWeaponByClass: %s is already equipped by %s."), *WeaponClass->GetName(), *OwningCharacter->GetName());
        return;
    }

    if (IsSwitchingWeapon())
    {
        UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerEquipWeaponByClass: %s - Already switching weapon, request to equip %s ignored."), *OwningCharacter->GetName(), *WeaponClass->GetName());
        return;
    }

    PendingWeapon = WeaponToEquip;
    float SwitchTime = 0.5f;
    const US_WeaponDataAsset* CurrentWeaponData = CurrentWeapon ? CurrentWeapon->GetWeaponData() : nullptr;
    const US_WeaponDataAsset* PendingWeaponData = PendingWeapon ? PendingWeapon->GetWeaponData() : nullptr;

    if (CurrentWeaponData && CurrentWeaponData->UnequipTime > 0)
    {
        SwitchTime = CurrentWeaponData->UnequipTime;
    }
    else if (PendingWeaponData && PendingWeaponData->EquipTime > 0)
    {
        SwitchTime = PendingWeaponData->EquipTime;
    }
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerEquipWeaponByClass: %s determined switch time to %s: %f seconds."), *OwningCharacter->GetName(), *PendingWeapon->GetName(), SwitchTime);

    AS_Weapon* OldWeaponToUnequip = CurrentWeapon;
    if (CurrentWeapon)
    {
        UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerEquipWeaponByClass: %s - Starting unequip effects for current weapon %s."), *OwningCharacter->GetName(), *CurrentWeapon->GetName());
        CurrentWeapon->StartUnequipEffects();
    }

    GetWorld()->GetTimerManager().SetTimer(WeaponSwitchTimerHandle, this, &US_WeaponInventoryComponent::FinishWeaponSwitch, SwitchTime, false);
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerEquipWeaponByClass: %s initiating switch to %s. Timer set for %f seconds."), *OwningCharacter->GetName(), *PendingWeapon->GetName(), SwitchTime);
}

void US_WeaponInventoryComponent::FinishWeaponSwitch()
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::FinishWeaponSwitch: %s attempting to finish switch to %s. IsServer: %d"), *GetNameSafe(GetOwner()), *GetNameSafe(PendingWeapon), GetOwnerRole() == ROLE_Authority);
    if (GetOwnerRole() != ROLE_Authority || !OwningCharacter || !PendingWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent::FinishWeaponSwitch: Preconditions not met or PendingWeapon is null for %s."), *GetNameSafe(GetOwner()));
        PendingWeapon = nullptr;
        return;
    }

    AS_Weapon* OldWeapon = CurrentWeapon;

    if (OldWeapon && OldWeapon != PendingWeapon)
    {
        UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::FinishWeaponSwitch: %s - Fully unequipped old weapon %s."), *OwningCharacter->GetName(), *OldWeapon->GetName());
        OldWeapon->Unequip();
    }

    CurrentWeapon = PendingWeapon;
    PendingWeapon = nullptr;

    if (CurrentWeapon)
    {
        UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::FinishWeaponSwitch: %s - Equipping new weapon %s."), *OwningCharacter->GetName(), *CurrentWeapon->GetName());
        CurrentWeapon->Equip(OwningCharacter);
    }

    OnRep_CurrentWeapon();

    OnWeaponEquippedDelegate.Broadcast(CurrentWeapon, OldWeapon);
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::FinishWeaponSwitch: %s finished switch. Equipped: %s. OldWeapon: %s"), *OwningCharacter->GetName(), CurrentWeapon ? *CurrentWeapon->GetName() : TEXT("NONE"), OldWeapon ? *OldWeapon->GetName() : TEXT("NONE"));
}


void US_WeaponInventoryComponent::ServerEquipWeaponByIndex(int32 Index)
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerEquipWeaponByIndex: %s attempting to equip by index %d."), *GetNameSafe(GetOwner()), Index);
    if (GetOwnerRole() == ROLE_Authority && WeaponInventory.IsValidIndex(Index) && WeaponInventory[Index])
    {
        ServerEquipWeaponByClass(WeaponInventory[Index]->GetClass());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent::ServerEquipWeaponByIndex: Invalid index %d or weapon not found for %s."), Index, *GetNameSafe(GetOwner()));
    }
}

bool US_WeaponInventoryComponent::ServerRequestNextWeapon_Validate() { return true; }
void US_WeaponInventoryComponent::ServerRequestNextWeapon_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerRequestNextWeapon_Implementation: %s."), *GetNameSafe(GetOwner()));
    if (WeaponInventory.Num() <= 1 || IsSwitchingWeapon()) return;

    int32 CurrentIndex = CurrentWeapon ? WeaponInventory.IndexOfByKey(CurrentWeapon) : -1;
    int32 NextIndex = 0;
    if (CurrentIndex != INDEX_NONE)
    {
        NextIndex = (CurrentIndex + 1) % WeaponInventory.Num();
    }

    if (WeaponInventory.IsValidIndex(NextIndex) && WeaponInventory[NextIndex] != CurrentWeapon)
    {
        ServerEquipWeaponByClass(WeaponInventory[NextIndex]->GetClass());
    }
}

bool US_WeaponInventoryComponent::ServerRequestPreviousWeapon_Validate() { return true; }
void US_WeaponInventoryComponent::ServerRequestPreviousWeapon_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::ServerRequestPreviousWeapon_Implementation: %s."), *GetNameSafe(GetOwner()));
    if (WeaponInventory.Num() <= 1 || IsSwitchingWeapon()) return;

    int32 CurrentIndex = CurrentWeapon ? WeaponInventory.IndexOfByKey(CurrentWeapon) : -1;
    int32 PrevIndex = 0;
    if (CurrentIndex != INDEX_NONE)
    {
        PrevIndex = (CurrentIndex - 1 + WeaponInventory.Num()) % WeaponInventory.Num();
    }
    else if (WeaponInventory.Num() > 0)
    {
        PrevIndex = WeaponInventory.Num() - 1;
    }

    if (WeaponInventory.IsValidIndex(PrevIndex) && WeaponInventory[PrevIndex] != CurrentWeapon)
    {
        ServerEquipWeaponByClass(WeaponInventory[PrevIndex]->GetClass());
    }
}

void US_WeaponInventoryComponent::OnRep_WeaponInventory()
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::OnRep_WeaponInventory: Client %s - WeaponInventory replicated, Count: %d"), *GetNameSafe(GetOwner()), WeaponInventory.Num());
}

void US_WeaponInventoryComponent::OnRep_CurrentWeapon()
{
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::OnRep_CurrentWeapon: Client %s - Replicated CurrentWeapon: %s"), *GetNameSafe(GetOwner()), *GetNameSafe(CurrentWeapon));
    AS_Weapon* OldWeaponForDelegate = nullptr;

    AS_Character* LocalCharacter = Cast<AS_Character>(GetOwner());
    if (LocalCharacter)
    {
        for (AS_Weapon* Weapon : WeaponInventory)
        {
            if (Weapon && Weapon != CurrentWeapon && Weapon->IsEquipped())
            {
                UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::OnRep_CurrentWeapon: Client %s - Unequipping %s due to current weapon change."), *LocalCharacter->GetName(), *Weapon->GetName());
                Weapon->Unequip();
            }
        }
        if (CurrentWeapon && !CurrentWeapon->IsEquipped())
        {
            UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent::OnRep_CurrentWeapon: Client %s - Equipping new current weapon %s."), *LocalCharacter->GetName(), *CurrentWeapon->GetName());
            CurrentWeapon->Equip(LocalCharacter);
        }
    }

    OnWeaponEquippedDelegate.Broadcast(CurrentWeapon, OldWeaponForDelegate);
}

bool US_WeaponInventoryComponent::HasWeapon(TSubclassOf<AS_Weapon> WeaponClass) const
{
    if (!WeaponClass) return false;
    for (const AS_Weapon* Weapon : WeaponInventory)
    {
        if (Weapon && Weapon->IsA(WeaponClass))
        {
            return true;
        }
    }
    return false;
}

bool US_WeaponInventoryComponent::IsSwitchingWeapon() const
{
    return GetWorld() ? GetWorld()->GetTimerManager().IsTimerActive(WeaponSwitchTimerHandle) : false;
}