#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/S_ViewModelBase.h"
#include "GameplayEffectTypes.h" // For FOnAttributeChangeData
#include "S_WeaponViewModel.generated.h"

class AS_Weapon;
class US_WeaponDataAsset;
class UTexture2D;
class US_AttributeSet;
class UAbilitySystemComponent;

/**
 * ViewModel for representing a single weapon in the UI (e.g., in an inventory list or as the equipped weapon).
 */
UCLASS(BlueprintType)
class STRAFEGAME_API US_WeaponViewModel : public US_ViewModelBase
{
    GENERATED_BODY()

public:
    // Standard Initialize from base class (can be called by PlayerHUDViewModel if needed for base setup)
    virtual void Initialize(AS_PlayerController* InOwningPlayerController) override;

    // Custom Initialize that takes the weapon actor
    // This is NOT an override of the base Initialize due to different signature.
    // It's a new virtual function specific to WeaponViewModels.
    virtual void Initialize(AS_PlayerController* InOwningPlayerController, AS_Weapon* InWeaponActor);

    virtual void Deinitialize() override;

    //~ Begin Properties exposed to View (UMG)
    UPROPERTY(BlueprintReadOnly, Category = "WeaponViewModel")
    FText WeaponName;

    UPROPERTY(BlueprintReadOnly, Category = "WeaponViewModel")
    int32 CurrentAmmo;

    UPROPERTY(BlueprintReadOnly, Category = "WeaponViewModel")
    int32 MaxAmmo;

    UPROPERTY(BlueprintReadOnly, Category = "WeaponViewModel")
    float AmmoPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "WeaponViewModel")
    bool bIsEquipped;

    UPROPERTY(BlueprintReadOnly, Category = "WeaponViewModel")
    TSoftObjectPtr<UTexture2D> WeaponIcon;
    //~ End Properties exposed to View (UMG)

    AS_Weapon* GetWeaponActor() const { return WeaponActor.Get(); }
    void SetIsEquipped(bool bInIsEquipped);


    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponViewModelUpdated);
    UPROPERTY(BlueprintAssignable, Category = "WeaponViewModel|Events")
    FOnWeaponViewModelUpdated OnWeaponViewModelUpdated;

protected:
    UPROPERTY()
    TWeakObjectPtr<AS_Weapon> WeaponActor;

    UPROPERTY()
    TWeakObjectPtr<const US_WeaponDataAsset> WeaponData;

    UPROPERTY()
    TWeakObjectPtr<US_AttributeSet> PlayerAttributeSet;

    UPROPERTY()
    TWeakObjectPtr<UAbilitySystemComponent> PlayerAbilitySystemComponent;

    FGameplayAttribute CurrentAmmoAttribute;
    FGameplayAttribute MaxAmmoAttribute;

    FDelegateHandle CurrentAmmoChangedHandle;
    FDelegateHandle MaxAmmoChangedHandle;

    virtual void UpdateWeaponData();
    virtual void HandleCurrentAmmoChanged(const FOnAttributeChangeData& Data);
    virtual void HandleMaxAmmoChanged(const FOnAttributeChangeData& Data);

    virtual void BindToAmmoAttributes();
    virtual void UnbindFromAmmoAttributes();
};