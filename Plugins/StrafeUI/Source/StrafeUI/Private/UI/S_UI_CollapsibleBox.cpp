// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_CollapsibleBox.cpp

#include "UI/S_UI_CollapsibleBox.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

void US_UI_CollapsibleBox::NativePreConstruct()
{
	Super::NativePreConstruct();
	UpdateHeaderText();
}

void US_UI_CollapsibleBox::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (HeaderButton)
	{
		HeaderButton->OnClicked.AddDynamic(this, &US_UI_CollapsibleBox::OnHeaderClicked);
	}

	// Start collapsed
	if (ContentBox)
	{
		ContentBox->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void US_UI_CollapsibleBox::OnHeaderClicked()
{
	if (ContentBox)
	{
		const bool bIsCollapsed = ContentBox->GetVisibility() == ESlateVisibility::Collapsed;
		ContentBox->SetVisibility(bIsCollapsed ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void US_UI_CollapsibleBox::UpdateHeaderText()
{
	if (HeaderLabel)
	{
		HeaderLabel->SetText(HeaderText);
	}
}