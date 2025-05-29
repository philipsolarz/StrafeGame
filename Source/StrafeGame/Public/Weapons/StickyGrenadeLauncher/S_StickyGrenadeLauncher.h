// Source/StrafeGame/Public/Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncher.h
#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_ProjectileWeapon.h"
#include "S_StickyGrenadeLauncher.generated.h"

class US_StickyGrenadeLauncherDataAsset;
class AS_StickyGrenadeProjectile;

UCLASS(Blueprintable)
class STRAFEGAME_API AS_StickyGrenadeLauncher : public AS_ProjectileWeapon
{
    GENERATED_BODY()

public:
    AS_StickyGrenadeLauncher();

    //~ Begin AS_Weapon Interface
    virtual void ExecutePrimaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData) override;
    virtual void ExecuteSecondaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData) override;
    //~ End AS_Weapon Interface

    UFUNCTION(BlueprintCallable, Category = "StickyGrenadeLauncher")
    bool DetonateOldestActiveSticky();
};