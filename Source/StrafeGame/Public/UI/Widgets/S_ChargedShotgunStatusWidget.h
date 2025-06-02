#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S_ChargedShotgunStatusWidget.generated.h"

class UProgressBar;
class US_ChargedShotgunViewModel;

UCLASS()
class STRAFEGAME_API US_ChargedShotgunStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Charged Shotgun Status")
    void SetViewModel(US_ChargedShotgunViewModel* InViewModel);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModel")
    TObjectPtr<US_ChargedShotgunViewModel> ChargedShotgunViewModel;

    // Bind these in your WBP_ChargedShotgunStatus Blueprint
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UProgressBar> PBPrimaryCharge;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UProgressBar> PBSecondaryCharge;

    UFUNCTION()
    virtual void HandleViewModelUpdated();

    virtual void RefreshWidget();
};