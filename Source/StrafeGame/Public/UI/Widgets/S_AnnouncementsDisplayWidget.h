#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S_AnnouncementsDisplayWidget.generated.h"

class UTextBlock;
class US_AnnouncerViewModel;

UCLASS()
class STRAFEGAME_API US_AnnouncementsDisplayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Announcements Widget")
    void SetViewModel(US_AnnouncerViewModel* InViewModel);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModel")
    TObjectPtr<US_AnnouncerViewModel> AnnouncerViewModel;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TxtAnnouncement;
    // You might also bind a Border or SizeBox for animating visibility

    UFUNCTION()
    virtual void HandleViewModelUpdated();

    virtual void RefreshWidget();
};