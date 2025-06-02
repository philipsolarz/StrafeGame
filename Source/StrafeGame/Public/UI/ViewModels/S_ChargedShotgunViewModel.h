// Source/StrafeGame/Public/UI/ViewModels/S_ChargedShotgunViewModel.h
#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/S_WeaponViewModel.h"
#include "S_ChargedShotgunViewModel.generated.h"

class AS_ChargedShotgun;
class AS_Weapon;
class AS_PlayerController;

UCLASS()
class STRAFEGAME_API US_ChargedShotgunViewModel : public US_WeaponViewModel
{
    GENERATED_BODY()

public:
    virtual void Initialize(AS_PlayerController* InOwningPlayerController, AS_Weapon* InWeaponActor) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintReadOnly, Category = "ChargedShotgunViewModel")
    float PrimaryChargeProgress;

    UPROPERTY(BlueprintReadOnly, Category = "ChargedShotgunViewModel")
    float SecondaryChargeProgress;

protected:
    TWeakObjectPtr<AS_ChargedShotgun> ChargedShotgunActor;

    UFUNCTION()
    void HandlePrimaryChargeProgressChanged(float NewProgress);
    UFUNCTION()
    void HandleSecondaryChargeProgressChanged(float NewProgress);
};