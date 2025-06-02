#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/S_ViewModelBase.h"
#include "S_AnnouncerViewModel.generated.h"

UCLASS()
class STRAFEGAME_API US_AnnouncerViewModel : public US_ViewModelBase
{
    GENERATED_BODY()

public:
    virtual void Initialize(AS_PlayerController* InOwningPlayerController) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintReadOnly, Category = "AnnouncerViewModel")
    FText CurrentAnnouncementText;

    UPROPERTY(BlueprintReadOnly, Category = "AnnouncerViewModel")
    bool bIsAnnouncementVisible;

    // Called by game systems to show an announcement
    UFUNCTION(BlueprintCallable, Category = "AnnouncerViewModel")
    void ShowAnnouncement(const FText& Message, float Duration = 3.0f);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnnouncementChanged);
    UPROPERTY(BlueprintAssignable, Category = "AnnouncerViewModel|Events")
    FOnAnnouncementChanged OnAnnouncementChanged;

protected:
    FTimerHandle AnnouncementTimerHandle;
    void HideAnnouncement();
};