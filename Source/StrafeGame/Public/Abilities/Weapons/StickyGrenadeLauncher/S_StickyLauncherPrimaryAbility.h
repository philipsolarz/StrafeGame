#pragma once

#include "CoreMinimal.h"
#include "Abilities/Weapons/S_WeaponPrimaryAbility.h"
#include "S_StickyLauncherPrimaryAbility.generated.h"

class US_StickyGrenadeLauncherDataAsset;
class AS_StickyGrenadeLauncher;

/**
 * Primary fire ability for the Sticky Grenade Launcher.
 * Fires a single sticky grenade. Limited by MaxActiveProjectiles.
 */
UCLASS()
class STRAFEGAME_API US_StickyGrenadeLauncherPrimaryAbility : public US_WeaponPrimaryAbility
{
    GENERATED_BODY()

public:
    US_StickyGrenadeLauncherPrimaryAbility();

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
    virtual void PerformWeaponFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

    AS_StickyGrenadeLauncher* GetStickyLauncher() const;
    const US_StickyGrenadeLauncherDataAsset* GetStickyLauncherData() const;
};