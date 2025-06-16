// S_UI_TextButton.cpp

#include "UI/S_UI_TextButton.h"

void US_UI_TextButton::SetButtonText(const FText& InText)
{
    ButtonText = InText;
    if (Text_Label)
    {
        Text_Label->SetText(ButtonText);
    }
}

void US_UI_TextButton::NativePreConstruct()
{
    Super::NativePreConstruct();

    // If our Text_Label widget exists, update its text with the value
    // from our exposed ButtonText property.
    if (Text_Label)
    {
        Text_Label->SetText(ButtonText);
    }
}