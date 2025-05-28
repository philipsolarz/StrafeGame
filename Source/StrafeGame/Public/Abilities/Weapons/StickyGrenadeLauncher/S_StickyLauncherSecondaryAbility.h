#pragma once

#include "CoreMinimal.h"
#include "Abilities/Weapons/S_WeaponSecondaryAbility.h"
#include "S_StickyLauncherSecondaryAbility.generated.h"

class AS_StickyGrenadeLauncher;

/**
 * Secondary fire ability for the Sticky Grenade Launcher.
 * Manually detonates the oldest active sticky grenade.
 */
UCLASS()
class STRAFEGAME_API US_StickyGrenadeLauncherSecondaryAbility : public US_WeaponSecondaryAbility
{
    GENERATED_BODY()

public:
    US_StickyGrenadeLauncherSecondaryAbility();

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
    virtual void PerformWeaponSecondaryFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

    AS_StickyGrenadeLauncher* GetStickyLauncher() const;
};