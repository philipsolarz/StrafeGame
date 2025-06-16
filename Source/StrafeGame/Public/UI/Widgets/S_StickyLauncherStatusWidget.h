#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S_StickyLauncherStatusWidget.generated.h"

class UImage; // Forward-declare UImage
class US_StickyGrenadeLauncherViewModel;

UCLASS()
class STRAFEGAME_API US_StickyLauncherStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Sticky Launcher Status")
    void SetViewModel(US_StickyGrenadeLauncherViewModel* InViewModel);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModel")
    TObjectPtr<US_StickyGrenadeLauncherViewModel> StickyLauncherViewModel;

    // Bind these to the three point images in your UMG Widget
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> Img_Point1;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> Img_Point2;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> Img_Point3;

    UFUNCTION()
    virtual void HandleViewModelUpdated();

    virtual void RefreshWidget();
};