// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_ViewModelBase.cpp

#include "ViewModel/S_UI_ViewModelBase.h"

void US_UI_ViewModelBase::BroadcastDataChanged()
{
	if (OnDataChanged.IsBound())
	{
		OnDataChanged.Broadcast();
	}
}

