#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h" // Required for ATTRIBUTE_ACCESSORS
#include "S_AttributeSet.generated.h"

// Uses Unreal Macros to generate boilerplate code for an Attribute.
// Implements Get, Set, and Init functions for an FGameplayAttributeData.
//Expects UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttributeName, Category = "CategoryName")
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Base AttributeSet for players in StrafeGame.
 * Contains core attributes like Health, and can be expanded for Stamina, Mana, MovementSpeed, etc.
 * Also includes ammo attributes.
 */
UCLASS(Blueprintable, Config = Game) // Added Blueprintable if you want to create Blueprint child classes
class STRAFEGAME_API US_AttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    US_AttributeSet();

    //~ Begin UObject Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    //~ End UObject Interface

    //~ Begin UAttributeSet Interface
    /**
     * Called just before a GameplayEffect is executed to modify the base value of an attribute.
     * No more net-aware logic can be done here.
     * AttributeSet can be const during this call.
     */
    virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
    /**
     * Called just before a GameplayEffect is executed to modify the final value of an attribute.
     * No more net-aware logic can be done here.
     * AttributeSet can be const during this call.
     */
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    /**
     * Called just after a GameplayEffect is executed to modify the base value of an attribute.
     * This is a good place to perform clamping or responding to changes.
     */
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
    //~ End UAttributeSet Interface


    // --- Core Attributes ---

    // Current Health. Affected by Damage.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes|Health")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(US_AttributeSet, Health);

    // Maximum Health. Typically a buff/debuff target.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes|Health")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(US_AttributeSet, MaxHealth);

    // Current Stamina (Example)
    // UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Attributes|Stamina")
    // FGameplayAttributeData Stamina;
    // ATTRIBUTE_ACCESSORS(US_AttributeSet, Stamina);

    // Maximum Stamina (Example)
    // UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Attributes|Stamina")
    // FGameplayAttributeData MaxStamina;
    // ATTRIBUTE_ACCESSORS(US_AttributeSet, MaxStamina);

    // Movement Speed Multiplier (Example)
    // UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MovementSpeed, Category = "Attributes|Movement")
    // FGameplayAttributeData MovementSpeed;
    // ATTRIBUTE_ACCESSORS(US_AttributeSet, MovementSpeed);


    // --- Ammo Attributes ---
    // Note: Specific weapon ammo attributes. Consider if a more generic system is needed later.

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RocketAmmo, Category = "Attributes|Ammo")
    FGameplayAttributeData RocketAmmo;
    ATTRIBUTE_ACCESSORS(US_AttributeSet, RocketAmmo);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxRocketAmmo, Category = "Attributes|Ammo")
    FGameplayAttributeData MaxRocketAmmo;
    ATTRIBUTE_ACCESSORS(US_AttributeSet, MaxRocketAmmo);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StickyGrenadeAmmo, Category = "Attributes|Ammo")
    FGameplayAttributeData StickyGrenadeAmmo;
    ATTRIBUTE_ACCESSORS(US_AttributeSet, StickyGrenadeAmmo);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStickyGrenadeAmmo, Category = "Attributes|Ammo")
    FGameplayAttributeData MaxStickyGrenadeAmmo;
    ATTRIBUTE_ACCESSORS(US_AttributeSet, MaxStickyGrenadeAmmo);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ShotgunAmmo, Category = "Attributes|Ammo")
    FGameplayAttributeData ShotgunAmmo;
    ATTRIBUTE_ACCESSORS(US_AttributeSet, ShotgunAmmo);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxShotgunAmmo, Category = "Attributes|Ammo")
    FGameplayAttributeData MaxShotgunAmmo;
    ATTRIBUTE_ACCESSORS(US_AttributeSet, MaxShotgunAmmo);


    // --- Meta Attributes (Transient, used for damage calculation) ---

    // Damage is a meta attribute used to pass damage received from a GameplayEffect execution.
    // It's not replicated and only exists transiently.
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Damage", meta = (HideFromModifiers))
    FGameplayAttributeData Damage;
    ATTRIBUTE_ACCESSORS(US_AttributeSet, Damage);

    // Healing is a meta attribute for healing effects.
    // UPROPERTY(BlueprintReadOnly, Category = "Attributes|Healing", meta = (HideFromModifiers))
    // FGameplayAttributeData Healing;
    // ATTRIBUTE_ACCESSORS(US_AttributeSet, Healing);


protected:
    // --- OnRep Functions ---
    // These are called on clients when the replicated value changes.

    UFUNCTION()
    virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

    UFUNCTION()
    virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

    // UFUNCTION()
    // virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);
    // UFUNCTION()
    // virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);
    // UFUNCTION()
    // virtual void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed);

    UFUNCTION()
    virtual void OnRep_RocketAmmo(const FGameplayAttributeData& OldRocketAmmo);
    UFUNCTION()
    virtual void OnRep_MaxRocketAmmo(const FGameplayAttributeData& OldMaxRocketAmmo);

    UFUNCTION()
    virtual void OnRep_StickyGrenadeAmmo(const FGameplayAttributeData& OldStickyGrenadeAmmo);
    UFUNCTION()
    virtual void OnRep_MaxStickyGrenadeAmmo(const FGameplayAttributeData& OldMaxStickyGrenadeAmmo);

    UFUNCTION()
    virtual void OnRep_ShotgunAmmo(const FGameplayAttributeData& OldShotgunAmmo);
    UFUNCTION()
    virtual void OnRep_MaxShotgunAmmo(const FGameplayAttributeData& OldMaxShotgunAmmo);

    // Helper function to clamp an attribute to its min/max defined by other attributes.
    // (e.g., clamp Health between 0 and MaxHealth).
    // MaxValueAttribute can be nullptr if there's no upper bound attribute.
    virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue, const FGameplayAttribute* MaxValueAttribute = nullptr, float MinValue = 0.0f) const;

    // Helper function to adjust the attribute based on the change from a GameplayEffect.
    // This is commonly used for "meta" attributes like Damage.
    void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

};