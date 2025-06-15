// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_ReplayListEntryWidget.h

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "S_UI_ReplayListEntryWidget.generated.h"

class UCommonTextBlock;
class UImage;

/**
 * Widget representing a single replay file in the replays list
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_ReplayListEntryWidget : public UCommonUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    // IUserObjectListEntry interface
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
    // End of IUserObjectListEntry interface

protected:
    virtual void NativePreConstruct() override;

private:
    /** Gets the icon color based on file age */
    FLinearColor GetAgeColor(const FDateTime& Timestamp) const;

    // UI Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_FileName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_Date;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_Time;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_FileSize;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_ReplayIcon;

    // Cached replay data
    UPROPERTY()
    TWeakObjectPtr<class US_UI_VM_ReplayEntry> CachedReplayData;
};