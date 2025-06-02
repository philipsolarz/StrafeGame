#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S_StrafeStatusWidget.generated.h"

class UTextBlock;
class US_StrafeHUDViewModel;

UCLASS()
class STRAFEGAME_API US_StrafeStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Strafe Status Widget")
    void SetViewModel(US_StrafeHUDViewModel* InViewModel);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModel")
    TObjectPtr<US_StrafeHUDViewModel> StrafeHUDViewModel;

    // Bind UMG Elements
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TxtCurrentTime;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TxtBestTime;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TxtSplits; // Could be a list or formatted text

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TxtCheckpoints; // e.g. "CP: 1/5"

    UFUNCTION()
    virtual void HandleViewModelUpdated();

    virtual void RefreshWidget();
};