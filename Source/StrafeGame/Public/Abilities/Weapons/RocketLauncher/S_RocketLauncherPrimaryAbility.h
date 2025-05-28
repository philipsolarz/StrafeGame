#pragma once

#include "CoreMinimal.h"
#include "Abilities/Weapons/S_WeaponPrimaryAbility.h"
#include "S_RocketLauncherPrimaryAbility.generated.h"

class US_RocketLauncherDataAsset;
class AS_RocketLauncher;

/**
 * Primary fire ability for the Rocket Launcher.
 * Fires a single rocket projectile. Player can hold to "spam" fire based on weapon's rate of fire.
 */
UCLASS()
class STRAFEGAME_API US_RocketLauncherPrimaryAbility : public US_WeaponPrimaryAbility
{
    GENERATED_BODY()

public:
    US_RocketLauncherPrimaryAbility();

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
    virtual void PerformWeaponFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

    // Helper to get typed weapon and data asset
    AS_RocketLauncher* GetRocketLauncher() const;
    const US_RocketLauncherDataAsset* GetRocketLauncherData() const;
};