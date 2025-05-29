// Source/StrafeGame/Public/Weapons/ChargedShotgun/S_ChargedShotgun.h
#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_HitscanWeapon.h"
#include "S_ChargedShotgun.generated.h"

class US_ChargedShotgunDataAsset;

UCLASS(Blueprintable)
class STRAFEGAME_API AS_ChargedShotgun : public AS_HitscanWeapon
{
    GENERATED_BODY()

public:
    AS_ChargedShotgun();

    //~ Begin AS_Weapon Interface
    virtual void ExecutePrimaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData) override;
    virtual void ExecuteSecondaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData) override;
    //~ End AS_Weapon Interface

    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Primary Charge Start"))
    void K2_OnPrimaryChargeStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Primary Charge Complete"))
    void K2_OnPrimaryChargeComplete();

    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Primary Charge Cancelled"))
    void K2_OnPrimaryChargeCancelled();

    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Secondary Charge Start"))
    void K2_OnSecondaryChargeStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Secondary Charge Held (Fully Charged)"))
    void K2_OnSecondaryChargeHeld();

    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Secondary Charge Released (Fired)"))
    void K2_OnSecondaryChargeReleased();

    UFUNCTION(BlueprintImplementableEvent, Category = "ChargedShotgun|Effects", meta = (DisplayName = "On Secondary Charge Cancelled"))
    void K2_OnSecondaryChargeCancelled();
};