#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "S_WeaponAbility.generated.h"

// Forward Declarations
class AS_Weapon;
class AS_Character;
class US_WeaponDataAsset;
class UAnimMontage; // For TSoftObjectPtr<UAnimMontage>
class UAbilityTask_PlayMontageAndWait;

UCLASS(Abstract, Blueprintable)
class STRAFEGAME_API US_WeaponAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    US_WeaponAbility();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityConfig")
    int32 AbilityInputID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityConfig")
    bool bActivateOnEquip;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityConfig")
    bool bCancelOnUnequip;

    UFUNCTION(BlueprintPure, Category = "WeaponAbility|Context")
    AS_Character* GetOwningSCharacter() const;

    UFUNCTION(BlueprintPure, Category = "WeaponAbility|Context")
    AS_Weapon* GetEquippedWeapon() const;

    UFUNCTION(BlueprintPure, Category = "WeaponAbility|Context")
    const US_WeaponDataAsset* GetEquippedWeaponData() const;

protected:
    /**
     * Plays a montage on the character's mesh and waits for it to complete.
     * This version assumes a single montage asset is provided (e.g., from WeaponDataAsset).
     * @param MontageAsset TSoftObjectPtr to the UAnimMontage to play.
     * @param Rate Play rate.
     * @param StartSectionName Optional section to start playing from.
     * @param bStopWhenAbilityEnds If true, montage will be stopped if ability ends prematurely.
     * @return The AbilityTask_PlayMontageAndWait instance, or nullptr if failed to play.
     */
    UFUNCTION(BlueprintCallable, Category = "WeaponAbility|Animation", meta = (AutoCreateRefTerm = "StartSectionName"))
    UAbilityTask_PlayMontageAndWait* PlayWeaponMontage(
        TSoftObjectPtr<UAnimMontage> MontageAsset,
        float Rate = 1.0f,
        FName StartSectionName = NAME_None,
        bool bStopWhenAbilityEnds = true
    );

    UFUNCTION(BlueprintPure, Category = "WeaponAbility|Context")
    const FGameplayAbilityActorInfo* GetAbilityActorInfo() const;
};