// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_KeyBindingWidget.cpp

#include "UI/S_UI_KeyBindingWidget.h"
#include "CommonTextBlock.h"
#include "CommonButtonBase.h"
#include "Components/Border.h"
#include "Framework/Application/SlateApplication.h"

void US_UI_KeyBindingWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Btn_Bind)
    {
        Btn_Bind->OnClicked().AddUObject(this, &US_UI_KeyBindingWidget::HandleBindButtonClicked);
    }
}

void US_UI_KeyBindingWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UpdateDisplay();
}

void US_UI_KeyBindingWidget::SetKeyBinding(const FKey& InKey)
{
    CurrentKey = InKey;
    UpdateDisplay();
    OnKeyBindingChanged.Broadcast(ActionName, CurrentKey);
}

void US_UI_KeyBindingWidget::StartKeyCapture()
{
    bIsCapturingKey = true;

    // Update visual state
    if (Text_CurrentKey)
    {
        Text_CurrentKey->SetText(FText::FromString(TEXT("Press any key...")));
    }

    if (Border_Background)
    {
        // Highlight the widget to show it's in capture mode
        Border_Background->SetBrushColor(FLinearColor(0.2f, 0.8f, 0.2f, 1.0f));
    }

    // Set keyboard focus to this widget
    SetKeyboardFocus();
}

void US_UI_KeyBindingWidget::CancelKeyCapture()
{
    bIsCapturingKey = false;
    UpdateDisplay();
}

FReply US_UI_KeyBindingWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (bIsCapturingKey)
    {
        FKey Key = InKeyEvent.GetKey();

        // Ignore modifier keys by themselves
        if (Key != EKeys::LeftShift && Key != EKeys::RightShift &&
            Key != EKeys::LeftControl && Key != EKeys::RightControl &&
            Key != EKeys::LeftAlt && Key != EKeys::RightAlt &&
            Key != EKeys::LeftCommand && Key != EKeys::RightCommand)
        {
            // Handle escape specially - it cancels the capture
            if (Key == EKeys::Escape)
            {
                CancelKeyCapture();
            }
            else
            {
                FinishKeyCapture(Key);
            }
            return FReply::Handled();
        }
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply US_UI_KeyBindingWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bIsCapturingKey)
    {
        FKey Key = InMouseEvent.GetEffectingButton();
        FinishKeyCapture(Key);
        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void US_UI_KeyBindingWidget::HandleBindButtonClicked()
{
    StartKeyCapture();
}

void US_UI_KeyBindingWidget::UpdateDisplay()
{
    if (Text_ActionName)
    {
        Text_ActionName->SetText(ActionDisplayName);
    }

    if (Text_CurrentKey && !bIsCapturingKey)
    {
        FString KeyString = CurrentKey.IsValid() ? CurrentKey.GetDisplayName().ToString() : TEXT("None");
        Text_CurrentKey->SetText(FText::FromString(KeyString));
    }

    if (Border_Background)
    {
        // Reset to normal color
        Border_Background->SetBrushColor(FLinearColor::White);
    }
}

void US_UI_KeyBindingWidget::FinishKeyCapture(const FKey& InKey)
{
    bIsCapturingKey = false;
    SetKeyBinding(InKey);
}