#include "Items/Pickups/S_PowerupPickup.h"
#include "Player/S_Character.h"
#include "Player/S_PlayerState.h"     // To get ASC
#include "AbilitySystemComponent.h" // For applying GameplayEffects and checking tags
#include "GameplayEffect.h"         // For UGameplayEffect

AS_PowerupPickup::AS_PowerupPickup()
{
    PowerupEffectClass = nullptr;
    bPreventStacking = true; // Default to preventing stacking if a PowerupGrantedTag is set
    // PowerupGrantedTag is left None by default.
}

bool AS_PowerupPickup::CanBePickedUp_Implementation(AS_Character* Picker) const
{
    if (!Super::CanBePickedUp_Implementation(Picker))
    {
        return false;
    }

    if (!PowerupEffectClass) // If no effect to grant, usually shouldn't be picked up.
    {
        return false;
    }

    if (Picker && bPreventStacking && PowerupGrantedTag.IsValid())
    {
        AS_PlayerState* PS = Picker->GetPlayerState<AS_PlayerState>();
        if (PS)
        {
            UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
            if (ASC && ASC->HasMatchingGameplayTag(PowerupGrantedTag))
            {
                // UE_LOG(LogTemp, Log, TEXT("AS_PowerupPickup: %s cannot pick up %s, already has active powerup tag %s."),
                //      *Picker->GetName(), *GetName(), *PowerupGrantedTag.ToString());
                return false; // Already has this powerup active, and it doesn't stack
            }
        }
    }

    return true;
}

void AS_PowerupPickup::GivePickupTo_Implementation(AS_Character* Picker)
{
    if (!Picker || !PowerupEffectClass)
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
            // Can be configured on the pickup if some powerups grant stronger versions via higher level GEs.
            float EffectLevel = 1.0f;

            FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(PowerupEffectClass, EffectLevel, EffectContext);

            if (SpecHandle.IsValid())
            {
                ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
                UE_LOG(LogTemp, Log, TEXT("AS_PowerupPickup: Applied PowerupEffect %s to %s."), *PowerupEffectClass->GetName(), *Picker->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("AS_PowerupPickup: Failed to make spec for PowerupEffect %s for %s."), *PowerupEffectClass->GetName(), *Picker->GetName());
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AS_PowerupPickup::GivePickupTo: Picker %s's PlayerState has no AbilitySystemComponent."), *Picker->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_PowerupPickup::GivePickupTo: Picker %s has no AS_PlayerState."), *Picker->GetName());
    }
}