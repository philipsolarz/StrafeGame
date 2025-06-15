// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_ServerListEntry.h

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "S_UI_ServerListEntry.generated.h"

class UCommonTextBlock;
class UImage;
class UProgressBar;

/**
 * Widget representing a single server in the server browser list
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_ServerListEntry : public UCommonUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    // IUserObjectListEntry interface
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
    // End of IUserObjectListEntry interface

protected:
    virtual void NativePreConstruct() override;

private:
    /** Updates the visual state based on server capacity */
    void UpdateServerStatusVisuals();

    /** Formats the player count text */
    FText GetPlayerCountText(int32 CurrentPlayers, int32 MaxPlayers) const;

    /** Gets the color for ping display */
    FLinearColor GetPingColor(int32 Ping) const;

    /** Gets the color for server status */
    FLinearColor GetServerStatusColor(int32 CurrentPlayers, int32 MaxPlayers) const;

    // UI Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Txt_ServerName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Txt_GameMode;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Txt_MapName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Txt_PlayerCount;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Txt_Ping;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> Bar_ServerCapacity;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_PrivateIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_LANIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Img_PingIcon;

    // Cached server data
    UPROPERTY()
    TWeakObjectPtr<class US_UI_VM_ServerListEntry> CachedServerData;
};