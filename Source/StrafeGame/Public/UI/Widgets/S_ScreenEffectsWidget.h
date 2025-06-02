#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S_ScreenEffectsWidget.generated.h"

class UImage; // Or UBorder
class US_ScreenEffectsViewModel;

UCLASS()
class STRAFEGAME_API US_ScreenEffectsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Screen Effects Widget")
    void SetViewModel(US_ScreenEffectsViewModel* InViewModel);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModel")
    TObjectPtr<US_ScreenEffectsViewModel> ScreenEffectsViewModel;

    // Bind UMG Element for damage effect (e.g., a red vignette border)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UImage> DamageIndicatorImage; // Or UBorder

    UFUNCTION()
    virtual void HandleViewModelUpdated();

    virtual void RefreshWidget();
};