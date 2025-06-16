// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_GameplaySettingsTab.cpp

#include "UI/S_UI_GameplaySettingsTab.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "ViewModel/S_UI_VM_Settings.h"

void US_UI_GameplaySettingsTab::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Bind UI events
    if (Slider_FieldOfView)
    {
        Slider_FieldOfView->OnValueChanged.AddDynamic(this, &US_UI_GameplaySettingsTab::OnFieldOfViewChanged);
        Slider_FieldOfView->SetMinValue(60.0f);
        Slider_FieldOfView->SetMaxValue(120.0f);
        Slider_FieldOfView->SetStepSize(1.0f);
    }

    if (Chk_ShowFPS)
    {
        Chk_ShowFPS->OnCheckStateChanged.AddDynamic(this, &US_UI_GameplaySettingsTab::OnShowFPSChanged);
    }
}

void US_UI_GameplaySettingsTab::OnViewModelDataChanged()
{
    if (!ViewModel.IsValid())
    {
        return;
    }

    // Update Field of View
    if (Slider_FieldOfView)
    {
        Slider_FieldOfView->SetValue(ViewModel->FieldOfView);
        OriginalFieldOfView = ViewModel->FieldOfView;
    }

    if (Text_FieldOfViewValue)
    {
        Text_FieldOfViewValue->SetText(FText::AsNumber(FMath::RoundToInt(ViewModel->FieldOfView)));
    }

    // Update Show FPS
    if (Chk_ShowFPS)
    {
        Chk_ShowFPS->SetIsChecked(ViewModel->bShowFPSCounter);
        bOriginalShowFPS = ViewModel->bShowFPSCounter;
    }
}

void US_UI_GameplaySettingsTab::OnFieldOfViewChanged(float Value)
{
    if (ViewModel.IsValid())
    {
        ViewModel->FieldOfView = Value;

        if (Text_FieldOfViewValue)
        {
            Text_FieldOfViewValue->SetText(FText::AsNumber(FMath::RoundToInt(Value)));
        }
    }
}

void US_UI_GameplaySettingsTab::OnShowFPSChanged(bool bIsChecked)
{
    if (ViewModel.IsValid())
    {
        ViewModel->bShowFPSCounter = bIsChecked;
    }
}

void US_UI_GameplaySettingsTab::ApplySettings()
{
    if (ViewModel.IsValid())
    {
        OriginalFieldOfView = ViewModel->FieldOfView;
        bOriginalShowFPS = ViewModel->bShowFPSCounter;
    }
}

void US_UI_GameplaySettingsTab::RevertSettings()
{
    if (ViewModel.IsValid())
    {
        ViewModel->FieldOfView = OriginalFieldOfView;
        ViewModel->bShowFPSCounter = bOriginalShowFPS;

        // Update UI
        OnViewModelDataChanged();
    }
}

bool US_UI_GameplaySettingsTab::HasUnsavedChanges() const
{
    if (!ViewModel.IsValid())
    {
        return false;
    }

    return FMath::Abs(ViewModel->FieldOfView - OriginalFieldOfView) > KINDA_SMALL_NUMBER ||
        ViewModel->bShowFPSCounter != bOriginalShowFPS;
}