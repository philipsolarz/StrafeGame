#include "Items/Pickups/S_ArmorPickup.h"
#include "Player/S_Character.h"
#include "Player/S_PlayerState.h"
#include "Player/Attributes/S_AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

AS_ArmorPickup::AS_ArmorPickup()
{
    ArmorGrantEffect = nullptr;
}

bool AS_ArmorPickup::CanBePickedUp_Implementation(AS_Character* Picker) const
{
    if (!Super::CanBePickedUp_Implementation(Picker))
    {
        return false;
    }

    if (!ArmorGrantEffect || !Picker)
    {
        return false;
    }

    // You would add Armor/MaxArmor to your AttributeSet and check it here.
    // For now, we'll assume it can always be picked up if not full.
    // Example:
    // if (const US_AttributeSet* AttributeSet = Picker->GetPlayerAttributeSet())
    // {
    //     if (AttributeSet->GetArmor() >= AttributeSet->GetMaxArmor())
    //     {
    //         return false;
    //     }
    // }

    return true;
}

void AS_ArmorPickup::GivePickupTo_Implementation(AS_Character* Picker)
{
    if (!Picker || !ArmorGrantEffect)
    {
        return;
    }

    if (UAbilitySystemComponent* ASC = Picker->GetPlayerAbilitySystemComponent())
    {
        FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
        EffectContext.AddSourceObject(this);
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ArmorGrantEffect, 1.0f, EffectContext);

        if (SpecHandle.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
    }
}