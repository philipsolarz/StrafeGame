#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/S_ViewModelBase.h"
#include "GameplayEffectTypes.h" // For FOnAttributeChangeData
#include "S_PlayerHUDViewModel.generated.h"

class US_AttributeSet;
class US_WeaponInventoryComponent;
class US_WeaponViewModel;
class AS_Weapon;
class UAbilitySystemComponent;

/**
 * ViewModel for the main player-specific HUD elements like health, armor, and weapon inventory.
 */
UCLASS()
class STRAFEGAME_API US_PlayerHUDViewModel : public US_ViewModelBase
{
    GENERATED_BODY()

public:
    virtual void Initialize(AS_PlayerController* InOwningPlayerController) override;
    virtual void Deinitialize() override;

    //~ Begin Properties exposed to View (UMG)
    UPROPERTY(BlueprintReadOnly, Category = "PlayerHUD|Attributes")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "PlayerHUD|Attributes")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "PlayerHUD|Attributes")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "PlayerHUD|Attributes")
    float CurrentArmor; // Assuming Armor is an attribute

    UPROPERTY(BlueprintReadOnly, Category = "PlayerHUD|Attributes")
    float MaxArmor;     // Assuming Armor is an attribute

    UPROPERTY(BlueprintReadOnly, Category = "PlayerHUD|Attributes")
    float ArmorPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "PlayerHUD|Weapons")
    TArray<TObjectPtr<US_WeaponViewModel>> WeaponInventoryViewModels;

    UPROPERTY(BlueprintReadOnly, Category = "PlayerHUD|Weapons")
    TObjectPtr<US_WeaponViewModel> EquippedWeaponViewModel;
    //~ End Properties exposed to View (UMG)

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnViewModelUpdated);
    UPROPERTY(BlueprintAssignable, Category = "PlayerHUD|Events")
    FOnViewModelUpdated OnViewModelUpdated;

protected:
    // Cached references to Model components
    UPROPERTY()
    TWeakObjectPtr<US_AttributeSet> PlayerAttributeSet;

    UPROPERTY()
    TWeakObjectPtr<UAbilitySystemComponent> PlayerAbilitySystemComponent;

    UPROPERTY()
    TWeakObjectPtr<US_WeaponInventoryComponent> WeaponInventoryComponent;

    // Delegate handles for attribute changes
    FDelegateHandle HealthChangedDelegateHandle;
    FDelegateHandle MaxHealthChangedDelegateHandle;
    FDelegateHandle ArmorChangedDelegateHandle; // Example
    FDelegateHandle MaxArmorChangedDelegateHandle; // Example

    // Functions to update properties from Models
    void UpdateHealth();
    void UpdateMaxHealth();
    void UpdateArmor(); // Example
    void UpdateMaxArmor(); // Example
    void UpdateWeaponInventory();
    void UpdateEquippedWeapon();

    // Callbacks for attribute changes
    virtual void HandleHealthChanged(const FOnAttributeChangeData& Data);
    virtual void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
    virtual void HandleArmorChanged(const FOnAttributeChangeData& Data); // Example
    virtual void HandleMaxArmorChanged(const FOnAttributeChangeData& Data); // Example

    // Callbacks for inventory changes
    UFUNCTION()
    virtual void HandleWeaponEquipped(AS_Weapon* NewWeapon, AS_Weapon* OldWeapon);
    UFUNCTION()
    virtual void HandleWeaponAdded(TSubclassOf<AS_Weapon> WeaponClass);

    void RefreshAllData();
};