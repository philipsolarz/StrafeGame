// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/MainMenu/Widgets/S_ServerRowWidget.h" // Updated include
#include "Components/TextBlock.h"

void US_ServerRowWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    ItemData = Cast<US_ServerRowData>(ListItemObject);
    UpdateDisplay();
}

void US_ServerRowWidget::UpdateDisplay()
{
    if (!ItemData)
    {
        if (Txt_ServerName) Txt_ServerName->SetText(FText::GetEmpty());
        if (Txt_MapName) Txt_MapName->SetText(FText::GetEmpty());
        if (Txt_PlayerCount) Txt_PlayerCount->SetText(FText::GetEmpty());
        if (Txt_Ping) Txt_Ping->SetText(FText::GetEmpty());
        return;
    }

    if (Txt_ServerName)
    {
        Txt_ServerName->SetText(FText::FromString(ItemData->ServerName));
    }
    if (Txt_MapName)
    {
        Txt_MapName->SetText(FText::FromString(ItemData->MapName));
    }
    if (Txt_PlayerCount)
    {
        Txt_PlayerCount->SetText(FText::Format(NSLOCTEXT("ServerRow", "PlayerCountFmt", "{0}/{1}"), ItemData->CurrentPlayers, ItemData->MaxPlayers));
    }
    if (Txt_Ping)
    {
        Txt_Ping->SetText(FText::AsNumber(ItemData->Ping));
    }
}