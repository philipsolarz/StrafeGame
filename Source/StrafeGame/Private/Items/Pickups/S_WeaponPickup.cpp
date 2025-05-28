#include "Items/Pickups/S_WeaponPickup.h"
#include "Player/S_Character.h"
#include "Player/Components/S_WeaponInventoryComponent.h" // For adding weapon/ammo
#include "Weapons/S_Weapon.h"                             // For AS_Weapon class
#include "Weapons/S_WeaponDataAsset.h"         // For accessing InitialAmmoEffect

AS_WeaponPickup::AS_WeaponPickup()
{
    WeaponClassToGrant = nullptr;
}

bool AS_WeaponPickup::CanBePickedUp_Implementation(AS_Character* Picker) const
{
    if (!Super::CanBePickedUp_Implementation(Picker)) // Basic check from parent
    {
        return false;
    }
    return WeaponClassToGrant != nullptr; // Can be picked up if a valid weapon class is set
}

void AS_WeaponPickup::GivePickupTo_Implementation(AS_Character* Picker)
{
    if (!Picker || !WeaponClassToGrant)
    {
        return;
    }

    US_WeaponInventoryComponent* Inventory = Picker->GetWeaponInventoryComponent();
    if (Inventory)
    {
        // ServerAddWeapon will handle giving the weapon if new,
        // or re-applying its InitialAmmoEffect if the character already has it.
        bool bPickedUpSuccessfully = Inventory->ServerAddWeapon(WeaponClassToGrant);

        if (bPickedUpSuccessfully)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_WeaponPickup: %s picked up by %s."), *WeaponClassToGrant->GetName(), *Picker->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_WeaponPickup: Failed to grant/add ammo for %s to %s via inventory."), *WeaponClassToGrant->GetName(), *Picker->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_WeaponPickup::GivePickupTo: Picker %s has no US_WeaponInventoryComponent."), *Picker->GetName());
    }
}