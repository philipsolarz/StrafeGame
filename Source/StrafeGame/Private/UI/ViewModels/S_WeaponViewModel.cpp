#include "UI/ViewModels/S_WeaponViewModel.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Player/S_PlayerController.h"
#include "Player/S_PlayerState.h"
#include "Player/Attributes/S_AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Engine/Texture2D.h" // For UTexture2D

// Initialize with AS_PlayerController*
void US_WeaponViewModel::Initialize(AS_PlayerController* InOwningPlayerController)
{
    Super::Initialize(InOwningPlayerController); // Call base class Initialize

    // General initialization if needed, weapon-specific parts are in the other Initialize
}


// Custom Initialize that takes the weapon actor
// Note: Removed 'override' as the signature is different from US_ViewModelBase::Initialize
void US_WeaponViewModel::Initialize(AS_PlayerController* InOwningPlayerController, AS_Weapon* InWeaponActor)
{
    Super::Initialize(InOwningPlayerController); // Call base class Initialize
    WeaponActor = InWeaponActor;

    if (WeaponActor.IsValid())
    {
        WeaponData = WeaponActor->GetWeaponData();
    }

    AS_PlayerState* PS = GetOwningPlayerController() ? GetOwningPlayerController()->GetPlayerState<AS_PlayerState>() : nullptr;
    if (PS)
    {
        PlayerAttributeSet = PS->GetAttributeSet();
        PlayerAbilitySystemComponent = PS->GetAbilitySystemComponent();
    }

    UpdateWeaponData();
    BindToAmmoAttributes();
}

void US_WeaponViewModel::Deinitialize()
{
    UnbindFromAmmoAttributes();
    WeaponActor.Reset();
    WeaponData.Reset();
    PlayerAttributeSet.Reset();
    PlayerAbilitySystemComponent.Reset();
    Super::Deinitialize();
}

void US_WeaponViewModel::UpdateWeaponData()
{
    if (WeaponData.IsValid())
    {
        WeaponName = WeaponData->WeaponDisplayName;
        WeaponIcon = WeaponData->WeaponIcon;

        if (WeaponData->AmmoAttribute.IsValid())
        {
            CurrentAmmoAttribute = WeaponData->AmmoAttribute;
        }
        if (WeaponData->MaxAmmoAttribute.IsValid())
        {
            MaxAmmoAttribute = WeaponData->MaxAmmoAttribute;
        }
    }
    else
    {
        WeaponName = FText::FromString(TEXT("N/A"));
        WeaponIcon = nullptr;
    }

    if (PlayerAbilitySystemComponent.IsValid()) // Changed from PlayerAttributeSet
    {
        CurrentAmmo = CurrentAmmoAttribute.IsValid() ? PlayerAbilitySystemComponent->GetNumericAttribute(CurrentAmmoAttribute) : 0;
        MaxAmmo = MaxAmmoAttribute.IsValid() ? PlayerAbilitySystemComponent->GetNumericAttribute(MaxAmmoAttribute) : 0;
        AmmoPercentage = (MaxAmmo > 0) ? (static_cast<float>(CurrentAmmo) / MaxAmmo) : 0.0f;
    }
    else
    {
        CurrentAmmo = 0;
        MaxAmmo = 0;
        AmmoPercentage = 0.0f;
    }
    OnWeaponViewModelUpdated.Broadcast();
}


void US_WeaponViewModel::BindToAmmoAttributes()
{
    UnbindFromAmmoAttributes();

    if (PlayerAbilitySystemComponent.IsValid()) // Check ASC here
    {
        if (CurrentAmmoAttribute.IsValid())
        {
            CurrentAmmoChangedHandle = PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CurrentAmmoAttribute).AddUObject(this, &US_WeaponViewModel::HandleCurrentAmmoChanged);
            CurrentAmmo = PlayerAbilitySystemComponent->GetNumericAttribute(CurrentAmmoAttribute); // Use ASC
        }
        if (MaxAmmoAttribute.IsValid())
        {
            MaxAmmoChangedHandle = PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MaxAmmoAttribute).AddUObject(this, &US_WeaponViewModel::HandleMaxAmmoChanged);
            MaxAmmo = PlayerAbilitySystemComponent->GetNumericAttribute(MaxAmmoAttribute); // Use ASC
        }
        AmmoPercentage = (MaxAmmo > 0) ? (static_cast<float>(CurrentAmmo) / MaxAmmo) : 0.0f;
        OnWeaponViewModelUpdated.Broadcast();
    }
}

void US_WeaponViewModel::UnbindFromAmmoAttributes()
{
    if (PlayerAbilitySystemComponent.IsValid())
    {
        if (CurrentAmmoAttribute.IsValid() && CurrentAmmoChangedHandle.IsValid())
        {
            PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CurrentAmmoAttribute).Remove(CurrentAmmoChangedHandle);
            CurrentAmmoChangedHandle.Reset();
        }
        if (MaxAmmoAttribute.IsValid() && MaxAmmoChangedHandle.IsValid())
        {
            PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MaxAmmoAttribute).Remove(MaxAmmoChangedHandle);
            MaxAmmoChangedHandle.Reset();
        }
    }
}


void US_WeaponViewModel::HandleCurrentAmmoChanged(const FOnAttributeChangeData& Data)
{
    CurrentAmmo = Data.NewValue;
    AmmoPercentage = (MaxAmmo > 0) ? (static_cast<float>(CurrentAmmo) / MaxAmmo) : 0.0f;
    OnWeaponViewModelUpdated.Broadcast();
}

void US_WeaponViewModel::HandleMaxAmmoChanged(const FOnAttributeChangeData& Data)
{
    MaxAmmo = Data.NewValue;
    AmmoPercentage = (MaxAmmo > 0) ? (static_cast<float>(CurrentAmmo) / MaxAmmo) : 0.0f;
    OnWeaponViewModelUpdated.Broadcast();
}

void US_WeaponViewModel::SetIsEquipped(bool bInIsEquipped)
{
    if (bIsEquipped != bInIsEquipped)
    {
        bIsEquipped = bInIsEquipped;
        OnWeaponViewModelUpdated.Broadcast();
    }
}