// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_ServerListEntry.cpp

#include "UI/S_UI_ServerListEntry.h"
#include "ViewModel/S_UI_VM_ServerBrowser.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"

void US_UI_ServerListEntry::NativePreConstruct()
{
    Super::NativePreConstruct();

    // Hide icons by default in the designer
    if (Img_PrivateIcon)
    {
        Img_PrivateIcon->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (Img_LANIcon)
    {
        Img_LANIcon->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void US_UI_ServerListEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (US_UI_VM_ServerListEntry* ServerEntry = Cast<US_UI_VM_ServerListEntry>(ListItemObject))
    {
        CachedServerData = ServerEntry;
        const F_ServerInfo& ServerInfo = ServerEntry->ServerInfo;

        // Update server name
        if (Txt_ServerName)
        {
            Txt_ServerName->SetText(ServerInfo.ServerName);
        }

        // Update game mode
        if (Txt_GameMode)
        {
            Txt_GameMode->SetText(ServerInfo.GameMode);
        }

        // Update map name
        if (Txt_MapName)
        {
            Txt_MapName->SetText(FText::FromString(ServerInfo.CurrentMap));
        }

        // Update player count
        if (Txt_PlayerCount)
        {
            Txt_PlayerCount->SetText(GetPlayerCountText(ServerInfo.PlayerCount, ServerInfo.MaxPlayers));

            // Set color based on server capacity
            FLinearColor CountColor = GetServerStatusColor(ServerInfo.PlayerCount, ServerInfo.MaxPlayers);
            Txt_PlayerCount->SetColorAndOpacity(FSlateColor(CountColor));
        }

        // Update server capacity bar
        if (Bar_ServerCapacity)
        {
            float FillPercent = ServerInfo.MaxPlayers > 0 ?
                (float)ServerInfo.PlayerCount / (float)ServerInfo.MaxPlayers : 0.0f;
            Bar_ServerCapacity->SetPercent(FillPercent);

            // Set bar color based on capacity
            FLinearColor BarColor = GetServerStatusColor(ServerInfo.PlayerCount, ServerInfo.MaxPlayers);
            Bar_ServerCapacity->SetFillColorAndOpacity(BarColor);
        }

        // Update ping
        if (Txt_Ping)
        {
            Txt_Ping->SetText(FText::AsNumber(ServerInfo.Ping));

            // Set color based on ping quality
            FLinearColor PingColor = GetPingColor(ServerInfo.Ping);
            Txt_Ping->SetColorAndOpacity(FSlateColor(PingColor));
        }

        // Update ping icon color
        if (Img_PingIcon)
        {
            FLinearColor PingColor = GetPingColor(ServerInfo.Ping);
            Img_PingIcon->SetColorAndOpacity(PingColor);
        }

        // Show/hide private icon
        if (Img_PrivateIcon)
        {
            Img_PrivateIcon->SetVisibility(ServerInfo.bIsPrivate ?
                ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
        }

        // Show/hide LAN icon
        if (Img_LANIcon)
        {
            Img_LANIcon->SetVisibility(ServerInfo.bIsLAN ?
                ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
        }

        UpdateServerStatusVisuals();
    }
}

void US_UI_ServerListEntry::UpdateServerStatusVisuals()
{
    if (!CachedServerData.IsValid())
    {
        return;
    }

    const F_ServerInfo& ServerInfo = CachedServerData->ServerInfo;

    // Additional visual updates based on server status
    // For example, you might want to dim full servers or highlight nearly empty ones
    float WidgetOpacity = 1.0f;

    if (ServerInfo.PlayerCount >= ServerInfo.MaxPlayers)
    {
        // Slightly dim full servers
        WidgetOpacity = 0.7f;
    }
    else if (ServerInfo.PlayerCount == 0)
    {
        // Slightly dim empty servers
        WidgetOpacity = 0.8f;
    }

    SetRenderOpacity(WidgetOpacity);
}

FText US_UI_ServerListEntry::GetPlayerCountText(int32 CurrentPlayers, int32 MaxPlayers) const
{
    return FText::Format(NSLOCTEXT("ServerList", "PlayerCount", "{0}/{1}"),
        FText::AsNumber(CurrentPlayers),
        FText::AsNumber(MaxPlayers));
}

FLinearColor US_UI_ServerListEntry::GetPingColor(int32 Ping) const
{
    if (Ping < 50)
    {
        // Excellent ping - Green
        return FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
    }
    else if (Ping < 100)
    {
        // Good ping - Yellow-Green
        return FLinearColor(0.7f, 1.0f, 0.0f, 1.0f);
    }
    else if (Ping < 150)
    {
        // Moderate ping - Yellow
        return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
    }
    else if (Ping < 200)
    {
        // Poor ping - Orange
        return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
    }
    else
    {
        // Very poor ping - Red
        return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
    }
}

FLinearColor US_UI_ServerListEntry::GetServerStatusColor(int32 CurrentPlayers, int32 MaxPlayers) const
{
    if (MaxPlayers <= 0)
    {
        return FLinearColor::White;
    }

    float FillPercent = (float)CurrentPlayers / (float)MaxPlayers;

    if (FillPercent >= 1.0f)
    {
        // Full server - Red
        return FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);
    }
    else if (FillPercent >= 0.75f)
    {
        // Nearly full - Orange
        return FLinearColor(1.0f, 0.6f, 0.0f, 1.0f);
    }
    else if (FillPercent >= 0.5f)
    {
        // Half full - Yellow
        return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
    }
    else if (FillPercent > 0.0f)
    {
        // Has players - Green
        return FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
    }
    else
    {
        // Empty - Gray
        return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
    }
}