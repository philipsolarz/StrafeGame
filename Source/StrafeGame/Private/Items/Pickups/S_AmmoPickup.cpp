#include "Items/Pickups/S_AmmoPickup.h"
#include "Player/S_Character.h"
#include "Player/S_PlayerState.h"       // To get ASC and AttributeSet
#include "Player/Attributes/S_AttributeSet.h" // To get MaxAmmoAttribute
#include "AbilitySystemComponent.h"   // For applying GameplayEffects
#include "GameplayEffect.h"           // For UGameplayEffect

AS_AmmoPickup::AS_AmmoPickup()
{
    AmmoGrantEffect = nullptr;
    // AmmoAttributeToRefill is left unset by default, should be configured in Blueprint
    // to match the attribute targeted by AmmoGrantEffect for the CanBePickedUp check.
}

bool AS_AmmoPickup::CanBePickedUp_Implementation(AS_Character* Picker) const
{
    if (!Super::CanBePickedUp_Implementation(Picker))
    {
        return false;
    }

    if (!AmmoGrantEffect) // If no effect to grant, can't be picked up.
    {
        return false;
    }

    // If AmmoAttributeToRefill is specified, check if the player is already at max for that ammo type.
    if (Picker && AmmoAttributeToRefill.IsValid())
    {
        AS_PlayerState* PS = Picker->GetPlayerState<AS_PlayerState>();
        if (PS)
        {
            UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
            US_AttributeSet* AttributeSet = PS->GetAttributeSet(); // Assuming AS_PlayerState has a GetAttributeSet()

            if (ASC && AttributeSet)
            {
                const float CurrentAmmo = ASC->GetNumericAttribute(AmmoAttributeToRefill);
                float MaxAmmo = 0.f;

                // Need to find the corresponding MaxAmmo attribute.
                // This part can be tricky if attributes aren't named consistently
                // or if you don't have a direct mapping here.
                // For simplicity, we assume US_AttributeSet has a way to get the Max for a given Ammo attribute,
                // or the AmmoGrantEffect itself also sets/clamps against max.
                // A more robust CanBePickedUp might involve checking tags or if the GE would even have an effect.

                // Example: Manually find matching MaxAmmoAttribute (less ideal, better to have a system or map in AttributeSet)
                // This logic should ideally be more robust, perhaps by querying the AttributeSet for the associated MaxAmmo attribute.
                if (AmmoAttributeToRefill == AttributeSet->GetRocketAmmoAttribute()) MaxAmmo = ASC->GetNumericAttribute(AttributeSet->GetMaxRocketAmmoAttribute());
                else if (AmmoAttributeToRefill == AttributeSet->GetStickyGrenadeAmmoAttribute()) MaxAmmo = ASC->GetNumericAttribute(AttributeSet->GetMaxStickyGrenadeAmmoAttribute());
                else if (AmmoAttributeToRefill == AttributeSet->GetShotgunAmmoAttribute()) MaxAmmo = ASC->GetNumericAttribute(AttributeSet->GetMaxShotgunAmmoAttribute());
                // ... add other ammo types ...
                else
                {
                    // If no specific max ammo attribute found for this type, assume it can be picked up
                    // or that the GE handles any capping.
                    return true;
                }


                if (CurrentAmmo >= MaxAmmo && MaxAmmo > 0) // Only block if MaxAmmo is defined and non-zero
                {
                    // UE_LOG(LogTemp, Log, TEXT("AS_AmmoPickup: %s cannot pick up %s, already at max ammo (%f/%f)."),
                    //      *Picker->GetName(), *GetName(), CurrentAmmo, MaxAmmo);
                    return false; // Already at max ammo
                }
            }
        }
    }

    return true; // Default to true if no specific blocking condition met
}

void AS_AmmoPickup::GivePickupTo_Implementation(AS_Character* Picker)
{
    if (!Picker || !AmmoGrantEffect)
    {
        return;
    }

    AS_PlayerState* PS = Picker->GetPlayerState<AS_PlayerState>();
    if (PS)
    {
        UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
        if (ASC)
        {
            FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
            EffectContext.AddSourceObject(this); // This pickup actor is the source of the effect

            // Level of the effect, usually 1 for pickups.
            // Can be configured on the pickup if some ammo pickups grant more via higher level GEs.
            float EffectLevel = 1.0f;

            FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AmmoGrantEffect, EffectLevel, EffectContext);

            if (SpecHandle.IsValid())
            {
                ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
                UE_LOG(LogTemp, Log, TEXT("AS_AmmoPickup: Applied AmmoGrantEffect %s to %s."), *AmmoGrantEffect->GetName(), *Picker->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("AS_AmmoPickup: Failed to make spec for AmmoGrantEffect %s for %s."), *AmmoGrantEffect->GetName(), *Picker->GetName());
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AS_AmmoPickup::GivePickupTo: Picker %s's PlayerState has no AbilitySystemComponent."), *Picker->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_AmmoPickup::GivePickupTo: Picker %s has no AS_PlayerState."), *Picker->GetName());
    }
}