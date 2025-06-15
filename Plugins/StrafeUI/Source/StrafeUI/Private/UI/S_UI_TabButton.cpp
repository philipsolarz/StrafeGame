// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_TabButton.cpp

#include "UI/S_UI_TabButton.h"
#include "CommonTextBlock.h"

void US_UI_TabButton::SetTabLabelText(const FText& InText)
{
    CachedTabText = InText;
    if (TabLabel)
    {
        TabLabel->SetText(CachedTabText);
    }
}

FText US_UI_TabButton::GetTabLabelText() const
{
    return CachedTabText;
}

void US_UI_TabButton::NativeConstruct()
{
    Super::NativeConstruct();

    // Restore the cached text when the widget is constructed
    if (TabLabel && !CachedTabText.IsEmpty())
    {
        TabLabel->SetText(CachedTabText);
    }
}

void US_UI_TabButton::NativePreConstruct()
{
    Super::NativePreConstruct();

    // Apply any design-time text
    if (TabLabel && !CachedTabText.IsEmpty())
    {
        TabLabel->SetText(CachedTabText);
    }
}