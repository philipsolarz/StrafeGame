// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_LeaderboardEntryWidget.h

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "S_UI_LeaderboardEntryWidget.generated.h"

class UCommonTextBlock;
class UCommonButtonBase;
class UImage;

/**
 * Widget representing a single leaderboard entry
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_LeaderboardEntryWidget : public UCommonUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    // IUserObjectListEntry interface
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
    // End of IUserObjectListEntry interface

protected:
    virtual void NativeOnInitialized() override;

private:
    /** Called when the play replay button is clicked */
    UFUNCTION()
    void OnPlayReplayClicked();

    /** Gets the rank color based on position */
    FLinearColor GetRankColor(int32 Rank) const;

    // UI Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_Rank;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_PlayerName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_Time;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_PlayReplay;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_RankIcon;

    // Cached entry data
    UPROPERTY()
    TWeakObjectPtr<class US_UI_VM_LeaderboardEntry> CachedEntryData;

    // Cached view model reference
    UPROPERTY()
    TWeakObjectPtr<class US_UI_VM_Leaderboards> ParentViewModel;
};