// Source/StrafeGame/Public/Weapons/RocketLauncher/S_RocketLauncher.h
#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_ProjectileWeapon.h"
#include "S_RocketLauncher.generated.h"

class US_RocketLauncherDataAsset;

UCLASS(Blueprintable)
class STRAFEGAME_API AS_RocketLauncher : public AS_ProjectileWeapon
{
    GENERATED_BODY()

public:
    AS_RocketLauncher();

    //~ Begin AS_Weapon Interface
    virtual void ExecutePrimaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData) override;
    virtual void ExecuteSecondaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData) override;
    //~ End AS_Weapon Interface

    UFUNCTION(BlueprintCallable, Category = "RocketLauncher")
    bool DetonateOldestActiveRocket();
};