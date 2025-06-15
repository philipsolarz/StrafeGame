// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_SettingsTabBase.cpp

#include "UI/S_UI_SettingsTabBase.h"
#include "ViewModel/S_UI_VM_Settings.h"

void US_UI_SettingsTabBase::SetViewModel(US_UI_VM_Settings* InViewModel)
{
    if (InViewModel)
    {
        ViewModel = InViewModel;

        // Bind to data changes using AddDynamic for dynamic delegates
        if (ViewModel.IsValid())
        {
            ViewModel->OnDataChanged.AddDynamic(this, &US_UI_SettingsTabBase::OnViewModelDataChanged);
        }

        // Initial update
        OnViewModelDataChanged();
    }
}