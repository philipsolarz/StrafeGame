#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S_PlayerStatusWidget.generated.h"

class UTextBlock;
class UProgressBar;
class US_PlayerHUDViewModel;

UCLASS()
class STRAFEGAME_API US_PlayerStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Player Status Widget")
    void SetViewModel(US_PlayerHUDViewModel* InViewModel);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModel")
    TObjectPtr<US_PlayerHUDViewModel> PlayerHUDViewModel;

    // Bind these to UMG elements in your WBP_PlayerStatus Blueprint
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> TxtHealth;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UProgressBar> PBHealth;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> TxtArmor;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UProgressBar> PBArmor;

    UFUNCTION()
    virtual void HandleViewModelUpdated();

    virtual void RefreshWidget();
};
