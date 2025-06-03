#include "Player/Attributes/S_AttributeSet.h"
#include "Net/UnrealNetwork.h"        // For DOREPLIFETIME
#include "GameplayEffectExtension.h"  // For FGameplayEffectModCallbackData
#include "GameplayEffectTypes.h"      // For FGameplayEffectContextHandle (though not directly used here, good include for context)
// #include "YourGameModuleName/S_Character.h" // If you need to cast OwnerActor to your Character class

US_AttributeSet::US_AttributeSet()
{
    // Initialize default values here if necessary, though GameplayEffects are preferred for base values.
    InitHealth(100.0f);     // Initialize Health to 100
    InitMaxHealth(100.0f);  // Initialize MaxHealth to 100

    // Initialize other attributes as needed
    InitRocketAmmo(0.f);
    InitMaxRocketAmmo(999.f); // Example
    InitStickyGrenadeAmmo(0.f);
    InitMaxStickyGrenadeAmmo(999.f); // Example
    InitShotgunAmmo(0.f);
    InitMaxShotgunAmmo(999.f); // Example
}

void US_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Core Attributes
    DOREPLIFETIME_CONDITION_NOTIFY(US_AttributeSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(US_AttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    // DOREPLIFETIME_CONDITION_NOTIFY(US_AttributeSet, Stamina, COND_None, REPNOTIFY_Always);
    // DOREPLIFETIME_CONDITION_NOTIFY(US_AttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
    // DOREPLIFETIME_CONDITION_NOTIFY(US_AttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);

    // Ammo Attributes
    DOREPLIFETIME_CONDITION_NOTIFY(US_AttributeSet, RocketAmmo, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(US_AttributeSet, MaxRocketAmmo, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(US_AttributeSet, StickyGrenadeAmmo, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(US_AttributeSet, MaxStickyGrenadeAmmo, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(US_AttributeSet, ShotgunAmmo, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(US_AttributeSet, MaxShotgunAmmo, COND_None, REPNOTIFY_Always);

    // Note: Meta attributes like 'Damage' are not replicated.
}

void US_AttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    Super::PreAttributeBaseChange(Attribute, NewValue);

    // This function is called before an attribute's base value is changed.
    // This is a good place to clamp the "base" value before any modifiers are applied.
    // For example, if you had an XP attribute and wanted to ensure its base value never goes below 0.
    // ClampAttribute(Attribute, NewValue, nullptr); // Example generic clamp
}

void US_AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    // This function is called before an attribute's "current" value is changed,
    // after GEs are applied but before the final value is set.
    // This is the main place for clamping attributes to their min/max.

    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
    // else if (Attribute == GetStaminaAttribute())
    // {
    //     NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
    // }
    else if (Attribute == GetRocketAmmoAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxRocketAmmo());
    }
    else if (Attribute == GetStickyGrenadeAmmoAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStickyGrenadeAmmo());
    }
    else if (Attribute == GetShotgunAmmoAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShotgunAmmo());
    }
}

void US_AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
    UAbilitySystemComponent* SourceASC = Context.GetOriginalInstigatorAbilitySystemComponent();
    const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
    AActor* TargetActor = nullptr;
    AController* TargetController = nullptr;
    // AS_Character* TargetCharacter = nullptr; // Your character class
    if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
    {
        TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
        TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
        // TargetCharacter = Cast<AS_Character>(TargetActor);
    }

    // --- DAMAGE HANDLING ---
    if (Data.EvaluatedData.Attribute == GetDamageAttribute())
    {
        // Store a local copy of the amount of damage done and clear the Damage attribute.
        const float LocalDamageDone = GetDamage();
        SetDamage(0.f);

        if (LocalDamageDone > 0.0f)
        {
            // Apply the damage to Health
            SetHealth(FMath::Clamp(GetHealth() - LocalDamageDone, 0.0f, GetMaxHealth()));
            // UE_LOG(LogTemp, Log, TEXT("US_AttributeSet::PostGameplayEffectExecute: Applied %f damage. Health is now %f."), LocalDamageDone, GetHealth());

            // If TargetCharacter is valid, you could pass more info or play damage indications
            // if (TargetCharacter && !TargetCharacter->IsDead()) // Assuming IsDead() exists
            // {
            //     // TargetCharacter->PlayHitReact(Data.EffectSpec.GetContext().GetHitResult(), LocalDamageDone, SourceTags);
            // }
        }
    }
    // --- HEALING HANDLING (Example) ---
    // else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
    // {
    //     const float LocalHealingDone = GetHealing();
    //     SetHealing(0.f);
    //     if (LocalHealingDone > 0.0f)
    //     {
    //         SetHealth(FMath::Clamp(GetHealth() + LocalHealingDone, 0.0f, GetMaxHealth()));
    //     }
    // }
    // --- CURRENT VALUE CLAMPING (after effects that might push them out of bounds) ---
    // This can also be done in PreAttributeChange, but PostGameplayEffectExecute is also common
    // especially if an effect directly changes Health without going through Damage.
    else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
    }
    // else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
    // {
    //     SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
    // }
    else if (Data.EvaluatedData.Attribute == GetRocketAmmoAttribute())
    {
        SetRocketAmmo(FMath::Clamp(GetRocketAmmo(), 0.0f, GetMaxRocketAmmo()));
    }
    else if (Data.EvaluatedData.Attribute == GetStickyGrenadeAmmoAttribute())
    {
        SetStickyGrenadeAmmo(FMath::Clamp(GetStickyGrenadeAmmo(), 0.0f, GetMaxStickyGrenadeAmmo()));
    }
    else if (Data.EvaluatedData.Attribute == GetShotgunAmmoAttribute())
    {
        SetShotgunAmmo(FMath::Clamp(GetShotgunAmmo(), 0.0f, GetMaxShotgunAmmo()));
    }
    // --- ADJUST CURRENT VALUE IF MAX VALUE CHANGES ---
    // Example: If MaxHealth changes, ensure current Health doesn't exceed new MaxHealth.
    else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
    {
        AdjustAttributeForMaxChange(Health, MaxHealth, GetMaxHealth(), GetHealthAttribute());
    }
    // else if (Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
    // {
    //     AdjustAttributeForMaxChange(Stamina, MaxStamina, GetMaxStamina(), GetStaminaAttribute());
    // }
    else if (Data.EvaluatedData.Attribute == GetMaxRocketAmmoAttribute())
    {
        AdjustAttributeForMaxChange(RocketAmmo, MaxRocketAmmo, GetMaxRocketAmmo(), GetRocketAmmoAttribute());
    }
    else if (Data.EvaluatedData.Attribute == GetMaxStickyGrenadeAmmoAttribute())
    {
        AdjustAttributeForMaxChange(StickyGrenadeAmmo, MaxStickyGrenadeAmmo, GetMaxStickyGrenadeAmmo(), GetStickyGrenadeAmmoAttribute());
    }
    else if (Data.EvaluatedData.Attribute == GetMaxShotgunAmmoAttribute())
    {
        AdjustAttributeForMaxChange(ShotgunAmmo, MaxShotgunAmmo, GetMaxShotgunAmmo(), GetShotgunAmmoAttribute());
    }
}

void US_AttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue, const FGameplayAttribute* MaxValueAttribute, float MinValue) const
{
    if (MaxValueAttribute)
    {
        const float MaxValue = MaxValueAttribute->GetNumericValue(this); // 'this' is the AttributeSet instance
        NewValue = FMath::Clamp(NewValue, MinValue, MaxValue);
    }
    else
    {
        NewValue = FMath::Max(NewValue, MinValue);
    }
}

void US_AttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttributeToEdit, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
    UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
    const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
    if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && ASC)
    {
        // Change current value to maintain the CurrentValue / CurrentMaxValue percent
        const float CurrentValue = AffectedAttributeToEdit.GetCurrentValue();
        const float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

        ASC->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
    }
}


// --- OnRep Functions ---
void US_AttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(US_AttributeSet, Health, OldHealth);
}

void US_AttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(US_AttributeSet, MaxHealth, OldMaxHealth);
}

// void US_AttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
// {
//     GAMEPLAYATTRIBUTE_REPNOTIFY(US_AttributeSet, Stamina, OldStamina);
// }
// void US_AttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
// {
//     GAMEPLAYATTRIBUTE_REPNOTIFY(US_AttributeSet, MaxStamina, OldMaxStamina);
// }
// void US_AttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
// {
//     GAMEPLAYATTRIBUTE_REPNOTIFY(US_AttributeSet, MovementSpeed, OldMovementSpeed);
// }

void US_AttributeSet::OnRep_RocketAmmo(const FGameplayAttributeData& OldRocketAmmo)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(US_AttributeSet, RocketAmmo, OldRocketAmmo);
}
void US_AttributeSet::OnRep_MaxRocketAmmo(const FGameplayAttributeData& OldMaxRocketAmmo)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(US_AttributeSet, MaxRocketAmmo, OldMaxRocketAmmo);
}

void US_AttributeSet::OnRep_StickyGrenadeAmmo(const FGameplayAttributeData& OldStickyGrenadeAmmo)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(US_AttributeSet, StickyGrenadeAmmo, OldStickyGrenadeAmmo);
}
void US_AttributeSet::OnRep_MaxStickyGrenadeAmmo(const FGameplayAttributeData& OldMaxStickyGrenadeAmmo)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(US_AttributeSet, MaxStickyGrenadeAmmo, OldMaxStickyGrenadeAmmo);
}

void US_AttributeSet::OnRep_ShotgunAmmo(const FGameplayAttributeData& OldShotgunAmmo)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(US_AttributeSet, ShotgunAmmo, OldShotgunAmmo);
}
void US_AttributeSet::OnRep_MaxShotgunAmmo(const FGameplayAttributeData& OldMaxShotgunAmmo)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(US_AttributeSet, MaxShotgunAmmo, OldMaxShotgunAmmo);
}