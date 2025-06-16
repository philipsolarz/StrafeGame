#include "Items/Pickups/S_HealthPickup.h"
#include "Player/S_Character.h"
#include "Player/S_PlayerState.h"
#include "Player/Attributes/S_AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

AS_HealthPickup::AS_HealthPickup()
{
    HealthGrantEffect = nullptr;
}

bool AS_HealthPickup::CanBePickedUp_Implementation(AS_Character* Picker) const
{
    if (!Super::CanBePickedUp_Implementation(Picker))
    {
        return false;
    }

    if (!HealthGrantEffect || !Picker)
    {
        return false;
    }

    // Check if player's health is already at max
    if (const US_AttributeSet* AttributeSet = Picker->GetPlayerAttributeSet())
    {
        if (AttributeSet->GetHealth() >= AttributeSet->GetMaxHealth())
        {
            return false;
        }
    }

    return true;
}

void AS_HealthPickup::GivePickupTo_Implementation(AS_Character* Picker)
{
    if (!Picker || !HealthGrantEffect)
    {
        return;
    }

    if (UAbilitySystemComponent* ASC = Picker->GetPlayerAbilitySystemComponent())
    {
        FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
        EffectContext.AddSourceObject(this);
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(HealthGrantEffect, 1.0f, EffectContext);

        if (SpecHandle.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
    }
}