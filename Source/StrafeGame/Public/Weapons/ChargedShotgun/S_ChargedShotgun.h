// Source/StrafeGame/Public/Weapons/ChargedShotgun/S_ChargedShotgun.h
#pragma once

#include "CoreMinimal.h"
#include "Weapons/S_HitscanWeapon.h"
#include "S_ChargedShotgun.generated.h"

class US_ChargedShotgunDataAsset;

// Delegate for charge progress changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChargeProgressChanged, float, NewProgress);

UCLASS(Blueprintable)
class STRAFEGAME_API AS_ChargedShotgun : public AS_HitscanWeapon
{
    GENERATED_BODY()

public:
    AS_ChargedShotgun();

    //~ Begin AActor Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    //~ End AActor Interface

    //~ Begin AS_Weapon Interface
    virtual void ExecutePrimaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData) override;
    virtual void ExecuteSecondaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData) override;
    //~ End AS_Weapon Interface

    // --- Charge Progress Accessors ---
    UFUNCTION(BlueprintPure, Category = "ChargedShotgun|ChargeState")
    float GetPrimaryChargeProgress() const { return PrimaryChargeProgress; }

    UFUNCTION(BlueprintPure, Category = "ChargedShotgun|ChargeState")
    float GetSecondaryChargeProgress() const { return SecondaryChargeProgress; }

    UFUNCTION(BlueprintPure, Category = "ChargedShotgun|ChargeState")
    bool IsPrimaryCharging() const { return PrimaryChargeProgress > 0.0f && PrimaryChargeProgress < 1.0f; }

    UFUNCTION(BlueprintPure, Category = "ChargedShotgun|ChargeState")
    bool IsSecondaryCharging() const { return SecondaryChargeProgress > 0.0f && SecondaryChargeProgress < 1.0f; }

    UFUNCTION(BlueprintPure, Category = "ChargedShotgun|ChargeState")
    bool IsSecondaryFullyCharged() const { return SecondaryChargeProgress >= 1.0f; }

    // Called by abilities to update charge progress
    void SetPrimaryChargeProgress(float Progress);
    void SetSecondaryChargeProgress(float Progress);

    UPROPERTY(BlueprintAssignable, Category = "ChargedShotgun|Events")
    FOnChargeProgressChanged OnPrimaryChargeProgressChanged;

    UPROPERTY(BlueprintAssignable, Category = "ChargedShotgun|Events")
    FOnChargeProgressChanged OnSecondaryChargeProgressChanged;

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

protected:
    UPROPERTY(Transient, ReplicatedUsing = OnRep_PrimaryChargeProgress)
    float PrimaryChargeProgress;

    UPROPERTY(Transient, ReplicatedUsing = OnRep_SecondaryChargeProgress)
    float SecondaryChargeProgress;

    UFUNCTION()
    void OnRep_PrimaryChargeProgress();

    UFUNCTION()
    void OnRep_SecondaryChargeProgress();
};