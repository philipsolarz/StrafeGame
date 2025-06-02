#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S_ArenaStatusWidget.generated.h"

class UTextBlock;
class UListView;
class US_ArenaHUDViewModel;

UCLASS()
class STRAFEGAME_API US_ArenaStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Arena Status Widget")
    void SetViewModel(US_ArenaHUDViewModel* InViewModel);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModel")
    TObjectPtr<US_ArenaHUDViewModel> ArenaHUDViewModel;

    // Bind UMG Elements
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TxtTimeLimit;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TxtFragLimit;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TxtPlayerScore; // e.g., "Frags: X / Deaths: Y"

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TxtLeaderScore;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UListView> KillfeedListView;

    UFUNCTION()
    virtual void HandleViewModelUpdated();

    virtual void RefreshWidget();
    virtual void RefreshKillfeed();
};