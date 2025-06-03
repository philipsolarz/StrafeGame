// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainMenu/Widgets/S_CommonButton.h"
#include "Components/TextBlock.h"

US_CommonButton::US_CommonButton() { /* Constructor */ }

void US_CommonButton::SetButtonText(const FText& InText)
{
    if (ButtonTextBlock)
    {
        ButtonTextBlock->SetText(InText);
    }
}