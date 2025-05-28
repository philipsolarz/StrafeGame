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
    PrimaryComponentTick.bCanEverTick = false; // Not ticking by default
    SetIsReplicatedByDefault(true); // This component will replicate

    CurrentWeapon = nullptr;
    PendingWeapon = nullptr;
    OwningCharacter = nullptr;
    OwnerAbilitySystemComponent = nullptr;
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

    CacheOwnerReferences();

    if (GetOwnerRole() == ROLE_Authority && OwningCharacter)
    {
        // Grant starting weapons
        for (TSubclassOf<AS_Weapon> WeaponClass : StartingWeaponClasses)
        {
            if (WeaponClass)
            {
                ServerAddWeapon(WeaponClass);
            }
        }

        // Equip the first weapon in the starting list, or the first one added if any.
        if (StartingWeaponClasses.Num() > 0 && WeaponInventory.Num() > 0 && WeaponInventory[0]->IsA(StartingWeaponClasses[0]))
        {
            ServerEquipWeaponByClass(StartingWeaponClasses[0]);
        }
        else if (WeaponInventory.Num() > 0) // Fallback to equip whatever is first if starting list didn't match
        {
            ServerEquipWeaponByClass(WeaponInventory[0]->GetClass());
        }
    }
}

void US_WeaponInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up spawned weapon actors
    if (GetOwnerRole() == ROLE_Authority)
    {
        for (AS_Weapon* Weapon : WeaponInventory)
        {
            if (Weapon)
            {
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
    return OwningCharacter && OwnerAbilitySystemComponent;
}

AS_Weapon* US_WeaponInventoryComponent::SpawnWeaponActor(TSubclassOf<AS_Weapon> WeaponClass)
{
    if (!WeaponClass || GetOwnerRole() != ROLE_Authority || !GetWorld() || !OwningCharacter)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwningCharacter; // The Character owns the weapon actor
    SpawnParams.Instigator = OwningCharacter; // The Character is the instigator
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Spawn the weapon slightly offset or at a specific "holstered" location if desired,
    // then hide it until equipped.
    AS_Weapon* NewWeapon = GetWorld()->SpawnActor<AS_Weapon>(WeaponClass, OwningCharacter->GetActorLocation(), OwningCharacter->GetActorRotation(), SpawnParams);
    if (NewWeapon)
    {
        NewWeapon->SetActorHiddenInGame(true); // Hide until explicitly equipped
        NewWeapon->SetOwnerCharacter(OwningCharacter); // Let weapon know its character
    }
    return NewWeapon;
}

void US_WeaponInventoryComponent::ApplyInitialAmmoForWeapon(AS_Weapon* WeaponToGrantAmmo)
{
    if (GetOwnerRole() != ROLE_Authority || !WeaponToGrantAmmo || !CacheOwnerReferences()) return;

    const US_WeaponDataAsset* WeaponData = WeaponToGrantAmmo->GetWeaponData();
    if (OwnerAbilitySystemComponent && WeaponData && WeaponData->InitialAmmoEffect)
    {
        FGameplayEffectContextHandle EffectContext = OwnerAbilitySystemComponent->MakeEffectContext();
        EffectContext.AddSourceObject(WeaponToGrantAmmo); // Source of the effect is the weapon

        FGameplayEffectSpecHandle SpecHandle = OwnerAbilitySystemComponent->MakeOutgoingSpec(WeaponData->InitialAmmoEffect, 1, EffectContext);
        if (SpecHandle.IsValid())
        {
            OwnerAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent: Applied InitialAmmoEffect %s for weapon %s on %s"),
                *WeaponData->InitialAmmoEffect->GetName(), *WeaponToGrantAmmo->GetName(), *OwningCharacter->GetName());
        }
    }
    else
    {
        if (!WeaponData) 
        {
            UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent: Weapon %s has no WeaponData."), *WeaponToGrantAmmo->GetName());
        }
        else if (!WeaponData->InitialAmmoEffect) {
            UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent: WeaponData for %s has no InitialAmmoEffect set."), *WeaponToGrantAmmo->GetName());
        }
        else if (!OwnerAbilitySystemComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent: OwnerASC is null, cannot apply initial ammo for %s."), *WeaponToGrantAmmo->GetName());
        }
    }
}


bool US_WeaponInventoryComponent::ServerAddWeapon(TSubclassOf<AS_Weapon> WeaponClass)
{
    if (GetOwnerRole() != ROLE_Authority || !WeaponClass || !CacheOwnerReferences())
    {
        return false;
    }

    // Check if player already has this weapon type
    for (AS_Weapon* ExistingWeapon : WeaponInventory)
    {
        if (ExistingWeapon && ExistingWeapon->IsA(WeaponClass))
        {
            // Player already has this weapon. Grant ammo instead (as per pickup logic).
            UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent: Player %s already has %s. Applying initial ammo."), *OwningCharacter->GetName(), *WeaponClass->GetName());
            ApplyInitialAmmoForWeapon(ExistingWeapon); // Re-apply initial ammo effect (effectively "ammo pickup")
            OnWeaponAddedDelegate.Broadcast(WeaponClass); // Still broadcast, as pickup was "successful"
            return true;
        }
    }

    AS_Weapon* NewWeapon = SpawnWeaponActor(WeaponClass);
    if (NewWeapon)
    {
        WeaponInventory.Add(NewWeapon);
        OnRep_WeaponInventory(); // Notify clients of inventory change

        ApplyInitialAmmoForWeapon(NewWeapon); // Grant initial ammo

        OnWeaponAddedDelegate.Broadcast(WeaponClass);
        UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent: Added weapon %s to %s's inventory."), *NewWeapon->GetName(), *OwningCharacter->GetName());
        return true;
    }
    return false;
}

void US_WeaponInventoryComponent::ServerEquipWeaponByClass(TSubclassOf<AS_Weapon> WeaponClass)
{
    if (GetOwnerRole() != ROLE_Authority || !OwningCharacter) return;

    if (!WeaponClass) // Request to unequip
    {
        if (CurrentWeapon)
        {
            PendingWeapon = nullptr; // Clear any pending switch
            GetWorld()->GetTimerManager().ClearTimer(WeaponSwitchTimerHandle); // Stop existing switch

            AS_Weapon* OldWeapon = CurrentWeapon;
            CurrentWeapon->Unequip(); // Server-side unequip
            CurrentWeapon = nullptr;
            OnRep_CurrentWeapon(); // Replicate null CurrentWeapon
            OnWeaponEquippedDelegate.Broadcast(nullptr, OldWeapon); // Notify character/systems
            UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent: Unequipped weapon on %s."), *OwningCharacter->GetName());
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
        UE_LOG(LogTemp, Warning, TEXT("US_WeaponInventoryComponent: %s trying to equip %s, but not found in inventory."), *OwningCharacter->GetName(), *WeaponClass->GetName());
        return;
    }

    if (WeaponToEquip == CurrentWeapon && WeaponToEquip->IsEquipped())
    {
        // UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent: %s is already equipped."), *WeaponClass->GetName());
        return; // Already equipped
    }

    if (IsSwitchingWeapon())
    {
        // UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent: Already switching weapon, request to equip %s ignored."), *WeaponClass->GetName());
        return; // Already switching
    }

    PendingWeapon = WeaponToEquip;
    float SwitchTime = 0.5f; // Default switch time
    const US_WeaponDataAsset* CurrentWeaponData = CurrentWeapon ? CurrentWeapon->GetWeaponData() : nullptr;
    const US_WeaponDataAsset* PendingWeaponData = PendingWeapon ? PendingWeapon->GetWeaponData() : nullptr;

    if (CurrentWeaponData && CurrentWeaponData->UnequipTime > 0)
    {
        SwitchTime = CurrentWeaponData->UnequipTime;
    }
    else if (PendingWeaponData && PendingWeaponData->EquipTime > 0) // Use new weapon's data if current is null or has no specific time
    {
        SwitchTime = PendingWeaponData->EquipTime;
    }

    // If there's a weapon currently equipped, unequip it first (visually, abilities cleared on FinishWeaponSwitch)
    AS_Weapon* OldWeaponToUnequip = CurrentWeapon; // Keep track of this for the delegate
    if (CurrentWeapon)
    {
        CurrentWeapon->StartUnequipEffects(); // Play unequip anim/sound but don't fully unequip logical state yet
    }

    // Set timer for switch
    GetWorld()->GetTimerManager().SetTimer(WeaponSwitchTimerHandle, this, &US_WeaponInventoryComponent::FinishWeaponSwitch, SwitchTime, false);
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent: %s initiating switch to %s. Switch time: %f"), *OwningCharacter->GetName(), *PendingWeapon->GetName(), SwitchTime);
}

void US_WeaponInventoryComponent::FinishWeaponSwitch()
{
    if (GetOwnerRole() != ROLE_Authority || !OwningCharacter || !PendingWeapon)
    {
        PendingWeapon = nullptr; // Clear if something went wrong
        return;
    }

    AS_Weapon* OldWeapon = CurrentWeapon; // This was the weapon active before the switch started

    if (OldWeapon && OldWeapon != PendingWeapon)
    {
        OldWeapon->Unequip(); // Full unequip of the previous weapon
    }

    CurrentWeapon = PendingWeapon;
    PendingWeapon = nullptr; // Clear pending state

    if (CurrentWeapon)
    {
        CurrentWeapon->Equip(OwningCharacter); // Server-side equip
    }

    OnRep_CurrentWeapon(); // This will replicate CurrentWeapon to clients, triggering their OnRep_CurrentWeapon

    // Broadcast that the weapon equip is complete (after delay)
    // AS_Character will listen to this to grant/remove abilities
    OnWeaponEquippedDelegate.Broadcast(CurrentWeapon, OldWeapon);
    UE_LOG(LogTemp, Log, TEXT("US_WeaponInventoryComponent: %s finished switch. Equipped: %s"), *OwningCharacter->GetName(), CurrentWeapon ? *CurrentWeapon->GetName() : TEXT("NONE"));
}


void US_WeaponInventoryComponent::ServerEquipWeaponByIndex(int32 Index)
{
    if (GetOwnerRole() == ROLE_Authority && WeaponInventory.IsValidIndex(Index) && WeaponInventory[Index])
    {
        ServerEquipWeaponByClass(WeaponInventory[Index]->GetClass());
    }
}

bool US_WeaponInventoryComponent::ServerRequestNextWeapon_Validate() { return true; }
void US_WeaponInventoryComponent::ServerRequestNextWeapon_Implementation()
{
    if (WeaponInventory.Num() <= 1 || IsSwitchingWeapon()) return;

    int32 CurrentIndex = CurrentWeapon ? WeaponInventory.IndexOfByKey(CurrentWeapon) : -1; // Could be -1 if no weapon equipped
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
    if (WeaponInventory.Num() <= 1 || IsSwitchingWeapon()) return;

    int32 CurrentIndex = CurrentWeapon ? WeaponInventory.IndexOfByKey(CurrentWeapon) : -1;
    int32 PrevIndex = 0;
    if (CurrentIndex != INDEX_NONE)
    {
        PrevIndex = (CurrentIndex - 1 + WeaponInventory.Num()) % WeaponInventory.Num();
    }
    else if (WeaponInventory.Num() > 0) // If nothing equipped, "previous" could wrap to last
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
    // Client-side: Inventory list has changed. UI might need to update.
    // OwningCharacter = Cast<AS_Character>(GetOwner()); // Ensure owner is cached
    // UE_LOG(LogTemp, Log, TEXT("Client %s: WeaponInventory replicated, Count: %d"), OwningCharacter ? *OwningCharacter->GetName() : TEXT("UnknownOwner"), WeaponInventory.Num());
}

void US_WeaponInventoryComponent::OnRep_CurrentWeapon()
{
    // Client-side: The equipped weapon has changed.
    // AS_Character binds to OnWeaponEquippedDelegate to handle visual attachment and ability setup.
    // We need to find what the 'OldWeapon' was on the client to pass to the delegate.
    // This is tricky because OnRep_CurrentWeapon only tells us the new state.
    // A common approach is to store a "PreviousCurrentWeapon" locally on the client.

    AS_Weapon* OldWeaponForDelegate = nullptr; // This is a simplification. A robust solution might need more state.
    // For now, the primary purpose is to inform the character about the NEW weapon.
    // The character's HandleWeaponEquipped will primarily use NewWeapon.

// Ensure all other weapons are visually unequipped
// This should ideally be handled by AS_Weapon::Equip and AS_Weapon::Unequip internals
// and the AS_Character's response to OnWeaponEquippedDelegate.
    AS_Character* LocalCharacter = Cast<AS_Character>(GetOwner());
    if (LocalCharacter)
    {
        for (AS_Weapon* Weapon : WeaponInventory)
        {
            if (Weapon && Weapon != CurrentWeapon && Weapon->IsEquipped())
            {
                Weapon->Unequip(); // Visually unequip if it thought it was equipped
            }
        }
        if (CurrentWeapon && !CurrentWeapon->IsEquipped())
        {
            CurrentWeapon->Equip(LocalCharacter); // Visually equip the new weapon
        }
    }

    OnWeaponEquippedDelegate.Broadcast(CurrentWeapon, OldWeaponForDelegate);
    // UE_LOG(LogTemp, Log, TEXT("Client %s: OnRep_CurrentWeapon. New: %s"),
    //     LocalCharacter ? *LocalCharacter->GetName() : TEXT("UnknownOwner"),
    //     CurrentWeapon ? *CurrentWeapon->GetName() : TEXT("NONE"));
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