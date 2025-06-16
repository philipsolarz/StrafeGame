#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S_ChargedShotgunStatusWidget.generated.h"

class UImage; // Forward-declare UImage
class UMaterialInstanceDynamic; // Forward-declare the dynamic material class
class US_ChargedShotgunViewModel;

UCLASS()
class STRAFEGAME_API US_ChargedShotgunStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Charged Shotgun Status")
    void SetViewModel(US_ChargedShotgunViewModel* InViewModel);

protected:
    virtual void NativeConstruct() override; // Use NativeConstruct for material setup
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModel")
    TObjectPtr<US_ChargedShotgunViewModel> ChargedShotgunViewModel;

    // Bind these in your WBP_ChargedShotgunStatus Blueprint to UImage widgets
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> Img_PrimaryCharge;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> Img_SecondaryCharge;

    UFUNCTION()
    virtual void HandleViewModelUpdated();

    virtual void RefreshWidget();

private:
    // Store dynamic material instances for efficient updates
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> PrimaryChargeMID;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> SecondaryChargeMID;
};