// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_TabListWidget.cpp

#include "UI/S_UI_TabListWidget.h"
#include "Components/PanelWidget.h"
#include "CommonButtonBase.h"
#include "Logging/LogMacros.h"

void US_UI_TabListWidget::HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	// The base implementation is empty, but it's good practice to call it in case it's used in the future.
	Super::HandleTabCreation_Implementation(TabNameID, TabButton);

	if (ButtonContainer)
	{
		ButtonContainer->AddChild(TabButton);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("US_UI_TabListWidget: ButtonContainer is not bound! Tab buttons will not be visible."));
	}
}

void US_UI_TabListWidget::HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	Super::HandleTabRemoval_Implementation(TabNameID, TabButton);

	if (ButtonContainer && TabButton && ButtonContainer->HasChild(TabButton))
	{
		// CommonUI's tab list recycles tab buttons using a widget pool.
		// Simply removing the child from our container is the correct action here.
		// The base class handles the actual destruction or pooling of the widget.
		ButtonContainer->RemoveChild(TabButton);
	}
}