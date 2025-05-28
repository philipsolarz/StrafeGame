#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "S_WeaponInventoryComponent.generated.h"

// Forward Declarations
class AS_Weapon;
class AS_Character;
class US_WeaponDataAsset;
class UAbilitySystemComponent;

// Delegate when a new weapon is successfully added to the inventory (spawned and owned)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponAddedDelegate, TSubclassOf<AS_Weapon>, WeaponClass);

// Delegate when a weapon is equipped (after any switch delay)
// Passes both the new and the old weapon to allow for more complex unequip/equip logic if needed.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponEquippedDelegate, AS_Weapon*, NewWeapon, AS_Weapon*, OldWeapon);


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class STRAFEGAME_API US_WeaponInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    US_WeaponInventoryComponent();

    //~ Begin UActorComponent Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override; // Used to process StartingWeapons
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override; // Clean up spawned weapons
    //~ End UActorComponent Interface

    /**
     * Adds a weapon of the given class to the inventory.
     * Handles spawning the weapon actor and initializing its default ammo via a GameplayEffect.
     * This is a server-authoritative action.
     * @param WeaponClass The class of the weapon to add.
     * @return True if the weapon was successfully added (or if ammo was given for an existing weapon type), false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "WeaponInventory|Management", meta = (DisplayName = "Add Weapon (Server)"))
    bool ServerAddWeapon(TSubclassOf<AS_Weapon> WeaponClass);

    /**
     * Equips a weapon of the given class from the inventory.
     * Handles weapon switch delays. This is a server-authoritative action.
     * @param WeaponClass The class of the weapon to equip. If nullptr, unequips the current weapon.
     */
    UFUNCTION(BlueprintCallable, Category = "WeaponInventory|Management", meta = (DisplayName = "Equip Weapon by Class (Server)"))
    void ServerEquipWeaponByClass(TSubclassOf<AS_Weapon> WeaponClass);

    /**
     * Equips a weapon by its index in the inventory.
     * Server-authoritative.
     * @param Index The index of the weapon in the WeaponInventory array.
     */
    UFUNCTION(BlueprintCallable, Category = "WeaponInventory|Management", meta = (DisplayName = "Equip Weapon by Index (Server)"))
    void ServerEquipWeaponByIndex(int32 Index);

    /** Server RPC called by character input to equip the next weapon. */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRequestNextWeapon();

    /** Server RPC called by character input to equip the previous weapon. */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRequestPreviousWeapon();

    // --- Query Functions ---
    UFUNCTION(BlueprintPure, Category = "WeaponInventory|Query")
    AS_Weapon* GetCurrentWeapon() const { return CurrentWeapon; }

    UFUNCTION(BlueprintPure, Category = "WeaponInventory|Query")
    bool HasWeapon(TSubclassOf<AS_Weapon> WeaponClass) const;

    UFUNCTION(BlueprintPure, Category = "WeaponInventory|Query")
    const TArray<AS_Weapon*>& GetWeaponInventoryList() const { return WeaponInventory; }

    UFUNCTION(BlueprintPure, Category = "WeaponInventory|Query")
    bool IsSwitchingWeapon() const;

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "WeaponInventory|Events")
    FOnWeaponAddedDelegate OnWeaponAddedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "WeaponInventory|Events")
    FOnWeaponEquippedDelegate OnWeaponEquippedDelegate;

protected:
    /** List of weapon classes to grant to the character at the start of play. Processed on server. */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponInventory|Defaults")
    TArray<TSubclassOf<AS_Weapon>> StartingWeaponClasses;

    /** The list of actual weapon instances owned by the character. Replicated. */
    UPROPERTY(Transient, ReplicatedUsing = OnRep_WeaponInventory)
    TArray<TObjectPtr<AS_Weapon>> WeaponInventory;

    /** The currently equipped weapon. Replicated. */
    UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
    TObjectPtr<AS_Weapon> CurrentWeapon;

    /** Weapon that will be equipped after the switch timer finishes. Server-only. */
    UPROPERTY()
    TObjectPtr<AS_Weapon> PendingWeapon;

    /** Timer handle for managing weapon switch delays. Server-only. */
    FTimerHandle WeaponSwitchTimerHandle;

    UFUNCTION()
    virtual void OnRep_WeaponInventory();

    UFUNCTION()
    virtual void OnRep_CurrentWeapon();

    /**
     * Called on the server after the weapon switch delay to finalize equipping the PendingWeapon.
     */
    virtual void FinishWeaponSwitch();

    /**
     * Spawns and initializes a weapon actor of the given class.
     * Does not add it to inventory or equip it, just prepares it. Server-only.
     * @param WeaponClass Class to spawn.
     * @return The spawned weapon, or nullptr if failed.
     */
    AS_Weapon* SpawnWeaponActor(TSubclassOf<AS_Weapon> WeaponClass);

    /** Applies initial ammo GameplayEffect for the given weapon. Server-only. */
    void ApplyInitialAmmoForWeapon(AS_Weapon* WeaponToGrantAmmo);


    /** Cached owning character. */
    UPROPERTY()
    TObjectPtr<AS_Character> OwningCharacter;

    /** Cached owning player's AbilitySystemComponent. */
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> OwnerAbilitySystemComponent;

private:
    /** Helper to get the ASC from the owning character's player state. Returns true if successful. */
    bool CacheOwnerReferences();
};