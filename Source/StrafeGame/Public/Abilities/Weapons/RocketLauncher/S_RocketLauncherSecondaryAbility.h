#pragma once

#include "CoreMinimal.h"
#include "Abilities/Weapons/S_WeaponSecondaryAbility.h"
#include "S_RocketLauncherSecondaryAbility.generated.h"

class AS_RocketLauncher;

/**
 * Secondary fire ability for the Rocket Launcher.
 * Manually detonates the oldest active rocket projectile.
 */
UCLASS()
class STRAFEGAME_API US_RocketLauncherSecondaryAbility : public US_WeaponSecondaryAbility
{
    GENERATED_BODY()

public:
    US_RocketLauncherSecondaryAbility();

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
    virtual void PerformWeaponSecondaryFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

    AS_RocketLauncher* GetRocketLauncher() const;
};