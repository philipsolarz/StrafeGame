// Source/StrafeGame/Private/UI/ViewModels/S_PlayerHUDViewModel.cpp
#include "UI/ViewModels/S_PlayerHUDViewModel.h"
#include "Player/S_PlayerController.h"
#include "Player/S_PlayerState.h"
#include "Player/Attributes/S_AttributeSet.h"
#include "Player/Components/S_WeaponInventoryComponent.h"
#include "Player/S_Character.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h" // Include WeaponDataAsset
#include "UI/ViewModels/S_WeaponViewModel.h" 
#include "UI/ViewModels/S_ChargedShotgunViewModel.h" // For specific ViewModel creation
#include "UI/ViewModels/S_StickyGrenadeLauncherViewModel.h" // For specific ViewModel creation
#include "AbilitySystemComponent.h"

void US_PlayerHUDViewModel::Initialize(AS_PlayerController* InOwningPlayerController)
{
    Super::Initialize(InOwningPlayerController);

    AS_PlayerState* PS = GetOwningPlayerController() ? GetOwningPlayerController()->GetPlayerState<AS_PlayerState>() : nullptr;
    if (PS)
    {
        PlayerAttributeSet = PS->GetAttributeSet();
        PlayerAbilitySystemComponent = PS->GetAbilitySystemComponent();

        AS_Character* PlayerCharacter = PS->GetSCharacter();
        if (PlayerCharacter)
        {
            WeaponInventoryComponent = PlayerCharacter->GetWeaponInventoryComponent();
        }

        if (PlayerAbilitySystemComponent.IsValid() && PlayerAttributeSet.IsValid())
        {
            HealthChangedDelegateHandle = PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributeSet->GetHealthAttribute()).AddUObject(this, &US_PlayerHUDViewModel::HandleHealthChanged);
            MaxHealthChangedDelegateHandle = PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributeSet->GetMaxHealthAttribute()).AddUObject(this, &US_PlayerHUDViewModel::HandleMaxHealthChanged);
        }

        if (WeaponInventoryComponent.IsValid())
        {
            WeaponInventoryComponent->OnWeaponEquippedDelegate.AddDynamic(this, &US_PlayerHUDViewModel::HandleWeaponEquipped);
            WeaponInventoryComponent->OnWeaponAddedDelegate.AddDynamic(this, &US_PlayerHUDViewModel::HandleWeaponAdded);
        }
        RefreshAllData();
    }
}

void US_PlayerHUDViewModel::Deinitialize()
{
    if (PlayerAbilitySystemComponent.IsValid() && PlayerAttributeSet.IsValid())
    {
        if (HealthChangedDelegateHandle.IsValid())
            PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributeSet->GetHealthAttribute()).Remove(HealthChangedDelegateHandle);
        if (MaxHealthChangedDelegateHandle.IsValid())
            PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributeSet->GetMaxHealthAttribute()).Remove(MaxHealthChangedDelegateHandle);
    }
    if (WeaponInventoryComponent.IsValid())
    {
        WeaponInventoryComponent->OnWeaponEquippedDelegate.RemoveDynamic(this, &US_PlayerHUDViewModel::HandleWeaponEquipped);
        WeaponInventoryComponent->OnWeaponAddedDelegate.RemoveDynamic(this, &US_PlayerHUDViewModel::HandleWeaponAdded);
    }

    // Deinitialize and clear ViewModels
    for (US_WeaponViewModel* VM : WeaponInventoryViewModels)
    {
        if (VM) VM->Deinitialize();
    }
    WeaponInventoryViewModels.Empty();
    if (EquippedWeaponViewModel) EquippedWeaponViewModel->Deinitialize();
    EquippedWeaponViewModel = nullptr;

    Super::Deinitialize();
}

void US_PlayerHUDViewModel::RefreshAllData()
{
    UpdateHealth();
    UpdateMaxHealth();
    UpdateArmor();
    UpdateMaxArmor();
    UpdateWeaponInventory(); // This needs to be called before UpdateEquippedWeapon
    UpdateEquippedWeapon();
    OnViewModelUpdated.Broadcast();
}

void US_PlayerHUDViewModel::UpdateHealth()
{
    if (PlayerAttributeSet.IsValid())
    {
        CurrentHealth = PlayerAttributeSet->GetHealth();
        MaxHealth = PlayerAttributeSet->GetMaxHealth();
        HealthPercentage = (MaxHealth > 0) ? (CurrentHealth / MaxHealth) : 0.0f;
    }
}

void US_PlayerHUDViewModel::UpdateMaxHealth()
{
    if (PlayerAttributeSet.IsValid())
    {
        MaxHealth = PlayerAttributeSet->GetMaxHealth();
        HealthPercentage = (MaxHealth > 0) ? (CurrentHealth / MaxHealth) : 0.0f;
    }
}

void US_PlayerHUDViewModel::UpdateArmor()
{
    CurrentArmor = 0;
    ArmorPercentage = 0;
}

void US_PlayerHUDViewModel::UpdateMaxArmor()
{
    MaxArmor = 0;
    ArmorPercentage = 0;
}

void US_PlayerHUDViewModel::UpdateWeaponInventory()
{
    // Deinitialize old ViewModels first
    for (US_WeaponViewModel* VM : WeaponInventoryViewModels)
    {
        if (VM) VM->Deinitialize();
    }
    WeaponInventoryViewModels.Empty();

    if (WeaponInventoryComponent.IsValid() && GetOwningPlayerController())
    {
        const TArray<AS_Weapon*>& InventoryList = WeaponInventoryComponent->GetWeaponInventoryList();
        for (AS_Weapon* Weapon : InventoryList)
        {
            if (Weapon && Weapon->GetWeaponData())
            {
                TSubclassOf<US_WeaponViewModel> ViewModelClass = Weapon->GetWeaponData()->WeaponViewModelClass;
                if (!ViewModelClass) // Fallback to base if not specified
                {
                    ViewModelClass = US_WeaponViewModel::StaticClass();
                }

                US_WeaponViewModel* WeaponVM = NewObject<US_WeaponViewModel>(this, ViewModelClass);
                WeaponVM->Initialize(GetOwningPlayerController(), Weapon);
                WeaponInventoryViewModels.Add(WeaponVM);
            }
        }
    }
}

void US_PlayerHUDViewModel::UpdateEquippedWeapon()
{
    EquippedWeaponViewModel = nullptr; // Reset first

    if (WeaponInventoryComponent.IsValid())
    {
        AS_Weapon* EquippedWeaponActor = WeaponInventoryComponent->GetCurrentWeapon();

        // Set bIsEquipped on all inventory VMs
        for (US_WeaponViewModel* VM : WeaponInventoryViewModels)
        {
            if (VM)
            {
                VM->SetIsEquipped(VM->GetWeaponActor() == EquippedWeaponActor);
                if (VM->GetWeaponActor() == EquippedWeaponActor)
                {
                    EquippedWeaponViewModel = VM;
                }
            }
        }
    }
    // If EquippedWeaponViewModel is still null here but there's an equipped weapon,
    // it means UpdateWeaponInventory might not have created the correct ViewModel for it,
    // or the weapon isn't in the WeaponInventoryViewModels list.
    // The loop above should handle finding it if UpdateWeaponInventory ran correctly.
}


void US_PlayerHUDViewModel::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
    CurrentHealth = Data.NewValue;
    HealthPercentage = (MaxHealth > 0) ? (CurrentHealth / MaxHealth) : 0.0f;
    OnViewModelUpdated.Broadcast();
}

void US_PlayerHUDViewModel::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
    MaxHealth = Data.NewValue;
    HealthPercentage = (MaxHealth > 0) ? (CurrentHealth / MaxHealth) : 0.0f;
    OnViewModelUpdated.Broadcast();
}

void US_PlayerHUDViewModel::HandleArmorChanged(const FOnAttributeChangeData& Data)
{
    OnViewModelUpdated.Broadcast();
}

void US_PlayerHUDViewModel::HandleMaxArmorChanged(const FOnAttributeChangeData& Data)
{
    OnViewModelUpdated.Broadcast();
}

void US_PlayerHUDViewModel::HandleWeaponEquipped(AS_Weapon* NewWeapon, AS_Weapon* OldWeapon)
{
    // It's important that UpdateWeaponInventory is robust enough if a new weapon (not previously in list) is equipped.
    // The current UpdateWeaponInventory rebuilds the list, which is fine.
    UpdateWeaponInventory(); // Ensures all VMs are up-to-date or created
    UpdateEquippedWeapon();  // Sets the correct equipped VM and updates bIsEquipped flags
    OnViewModelUpdated.Broadcast();
}

void US_PlayerHUDViewModel::HandleWeaponAdded(TSubclassOf<AS_Weapon> WeaponClass)
{
    UpdateWeaponInventory();
    UpdateEquippedWeapon(); // Also update equipped status in case the added weapon was auto-equipped
    OnViewModelUpdated.Broadcast();
}