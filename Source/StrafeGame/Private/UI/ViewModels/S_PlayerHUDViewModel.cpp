#include "UI/ViewModels/S_PlayerHUDViewModel.h"
#include "Player/S_PlayerController.h"
#include "Player/S_PlayerState.h"
#include "Player/Attributes/S_AttributeSet.h"
#include "Player/Components/S_WeaponInventoryComponent.h"
#include "Player/S_Character.h"
#include "Weapons/S_Weapon.h"
#include "UI/ViewModels/S_WeaponViewModel.h" // Include the specific ViewModel
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
            // TODO: Bind to Armor attributes if they exist
            // ArmorChangedDelegateHandle = PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributeSet->GetArmorAttribute()).AddUObject(this, &US_PlayerHUDViewModel::HandleArmorChanged);
            // MaxArmorChangedDelegateHandle = PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributeSet->GetMaxArmorAttribute()).AddUObject(this, &US_PlayerHUDViewModel::HandleMaxArmorChanged);
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
        PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributeSet->GetHealthAttribute()).Remove(HealthChangedDelegateHandle);
        PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributeSet->GetMaxHealthAttribute()).Remove(MaxHealthChangedDelegateHandle);
        // TODO: Unbind Armor attributes
    }
    if (WeaponInventoryComponent.IsValid())
    {
        WeaponInventoryComponent->OnWeaponEquippedDelegate.RemoveDynamic(this, &US_PlayerHUDViewModel::HandleWeaponEquipped);
        WeaponInventoryComponent->OnWeaponAddedDelegate.RemoveDynamic(this, &US_PlayerHUDViewModel::HandleWeaponAdded);
    }

    WeaponInventoryViewModels.Empty();
    EquippedWeaponViewModel = nullptr;

    Super::Deinitialize();
}

void US_PlayerHUDViewModel::RefreshAllData()
{
    UpdateHealth();
    UpdateMaxHealth();
    UpdateArmor();
    UpdateMaxArmor();
    UpdateWeaponInventory();
    UpdateEquippedWeapon(); // This ensures equipped weapon VM is also created/updated
    OnViewModelUpdated.Broadcast();
}

void US_PlayerHUDViewModel::UpdateHealth()
{
    if (PlayerAttributeSet.IsValid())
    {
        CurrentHealth = PlayerAttributeSet->GetHealth();
        MaxHealth = PlayerAttributeSet->GetMaxHealth(); // Ensure MaxHealth is also up-to-date
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
    // TODO: Implement if Armor attribute exists
    // if (PlayerAttributeSet.IsValid())
    // {
    //     CurrentArmor = PlayerAttributeSet->GetArmor();
    //     ArmorPercentage = (MaxArmor > 0) ? (CurrentArmor / MaxArmor) : 0.0f;
    // }
    CurrentArmor = 0; // Placeholder
    ArmorPercentage = 0; // Placeholder
}

void US_PlayerHUDViewModel::UpdateMaxArmor()
{
    // TODO: Implement if MaxArmor attribute exists
    // if (PlayerAttributeSet.IsValid())
    // {
    //     MaxArmor = PlayerAttributeSet->GetMaxArmor();
    //     ArmorPercentage = (MaxArmor > 0) ? (CurrentArmor / MaxArmor) : 0.0f;
    // }
    MaxArmor = 0; // Placeholder
    ArmorPercentage = 0; // Placeholder
}

void US_PlayerHUDViewModel::UpdateWeaponInventory()
{
    WeaponInventoryViewModels.Empty(); // Clear existing
    if (WeaponInventoryComponent.IsValid() && GetOwningPlayerController())
    {
        const TArray<AS_Weapon*>& InventoryList = WeaponInventoryComponent->GetWeaponInventoryList();
        for (AS_Weapon* Weapon : InventoryList)
        {
            if (Weapon)
            {
                US_WeaponViewModel* WeaponVM = NewObject<US_WeaponViewModel>(this); // 'this' is the outer
                WeaponVM->Initialize(GetOwningPlayerController(), Weapon); // Custom Initialize for WeaponViewModel
                WeaponInventoryViewModels.Add(WeaponVM);
            }
        }
    }
}

void US_PlayerHUDViewModel::UpdateEquippedWeapon()
{
    if (WeaponInventoryComponent.IsValid() && GetOwningPlayerController())
    {
        AS_Weapon* EquippedWeaponActor = WeaponInventoryComponent->GetCurrentWeapon();
        if (EquippedWeaponActor)
        {
            // Find existing VM or create new one
            bool bFound = false;
            for (US_WeaponViewModel* VM : WeaponInventoryViewModels)
            {
                if (VM && VM->GetWeaponActor() == EquippedWeaponActor)
                {
                    EquippedWeaponViewModel = VM;
                    VM->SetIsEquipped(true);
                    bFound = true;
                }
                else if (VM)
                {
                    VM->SetIsEquipped(false);
                }
            }
            if (!bFound) // Should not happen if UpdateWeaponInventory was called first
            {
                US_WeaponViewModel* WeaponVM = NewObject<US_WeaponViewModel>(this);
                WeaponVM->Initialize(GetOwningPlayerController(), EquippedWeaponActor);
                WeaponVM->SetIsEquipped(true);
                EquippedWeaponViewModel = WeaponVM;
                // Add to inventory viewmodels if somehow missed
                if (!WeaponInventoryViewModels.Contains(WeaponVM))
                {
                    WeaponInventoryViewModels.Add(WeaponVM);
                }
            }
        }
        else
        {
            EquippedWeaponViewModel = nullptr;
            for (US_WeaponViewModel* VM : WeaponInventoryViewModels)
            {
                if (VM) VM->SetIsEquipped(false);
            }
        }
    }
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
    // TODO: Implement if Armor attribute exists
    // CurrentArmor = Data.NewValue;
    // ArmorPercentage = (MaxArmor > 0) ? (CurrentArmor / MaxArmor) : 0.0f;
    OnViewModelUpdated.Broadcast();
}

void US_PlayerHUDViewModel::HandleMaxArmorChanged(const FOnAttributeChangeData& Data)
{
    // TODO: Implement if MaxArmor attribute exists
    // MaxArmor = Data.NewValue;
    // ArmorPercentage = (MaxArmor > 0) ? (CurrentArmor / MaxArmor) : 0.0f;
    OnViewModelUpdated.Broadcast();
}

void US_PlayerHUDViewModel::HandleWeaponEquipped(AS_Weapon* NewWeapon, AS_Weapon* OldWeapon)
{
    // Full refresh might be easier here, or selectively update the equipped status
    UpdateWeaponInventory(); // This will re-create VMs if needed
    UpdateEquippedWeapon();
    OnViewModelUpdated.Broadcast();
}

void US_PlayerHUDViewModel::HandleWeaponAdded(TSubclassOf<AS_Weapon> WeaponClass)
{
    UpdateWeaponInventory(); // Rebuild the list of weapon VMs
    OnViewModelUpdated.Broadcast();
}