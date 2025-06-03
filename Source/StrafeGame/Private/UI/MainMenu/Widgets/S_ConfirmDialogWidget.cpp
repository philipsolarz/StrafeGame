// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/MainMenu/Widgets/S_ConfirmDialogWidget.h" // Updated include
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"

void US_ConfirmDialogWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Btn_Yes)
    {
        Btn_Yes->OnClicked().AddUObject(this, &US_ConfirmDialogWidget::OnYesClicked);
    }
    if (Btn_No)
    {
        Btn_No->OnClicked().AddUObject(this, &US_ConfirmDialogWidget::OnNoClicked);
    }
}

void US_ConfirmDialogWidget::SetupDialog(const FText& Title, const FText& Message)
{
    if (Txt_Title)
    {
        Txt_Title->SetText(Title);
    }
    if (Txt_Message)
    {
        Txt_Message->SetText(Message);
    }
}

void US_ConfirmDialogWidget::OnYesClicked()
{
    OnDialogClosedDelegate.Broadcast(true);
    DeactivateWidget();
}

void US_ConfirmDialogWidget::OnNoClicked()
{
    OnDialogClosedDelegate.Broadcast(false);
    DeactivateWidget();
}