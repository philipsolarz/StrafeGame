// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_VideoSettingsTab.cpp

#include "UI/S_UI_VideoSettingsTab.h"
#include "UI/S_UI_StringComboBox.h"
#include "Components/CheckBox.h"
#include "ViewModel/S_UI_VM_Settings.h"

void US_UI_VideoSettingsTab::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Bind UI events
    if (Cmb_Resolution)
    {
        Cmb_Resolution->OnSelectionChanged.AddDynamic(this, &US_UI_VideoSettingsTab::OnResolutionChanged);
    }
    if (Cmb_WindowMode)
    {
        Cmb_WindowMode->OnSelectionChanged.AddDynamic(this, &US_UI_VideoSettingsTab::OnWindowModeChanged);
    }
    if (Chk_VSync)
    {
        Chk_VSync->OnCheckStateChanged.AddDynamic(this, &US_UI_VideoSettingsTab::OnVSyncChanged);
    }
    if (Cmb_ShadowQuality)
    {
        Cmb_ShadowQuality->OnSelectionChanged.AddDynamic(this, &US_UI_VideoSettingsTab::OnShadowQualityChanged);
    }
    if (Cmb_TextureQuality)
    {
        Cmb_TextureQuality->OnSelectionChanged.AddDynamic(this, &US_UI_VideoSettingsTab::OnTextureQualityChanged);
    }
    if (Cmb_AntiAliasing)
    {
        Cmb_AntiAliasing->OnSelectionChanged.AddDynamic(this, &US_UI_VideoSettingsTab::OnAntiAliasingChanged);
    }
}

void US_UI_VideoSettingsTab::OnViewModelDataChanged()
{
    if (!ViewModel.IsValid())
    {
        return;
    }

    // Make sure video options are populated
    ViewModel->PopulateVideoOptions();

    // Update Resolution combo box
    if (Cmb_Resolution)
    {
        Cmb_Resolution->ClearOptions();
        for (const FString& ResOption : ViewModel->ResolutionOptions)
        {
            Cmb_Resolution->AddOption(ResOption);
        }
        if (ViewModel->ResolutionOptions.IsValidIndex(ViewModel->ResolutionIndex))
        {
            Cmb_Resolution->SetSelectedOption(ViewModel->ResolutionOptions[ViewModel->ResolutionIndex]);
        }
        OriginalResolutionIndex = ViewModel->ResolutionIndex;
    }

    // Update Window Mode combo box
    if (Cmb_WindowMode)
    {
        Cmb_WindowMode->ClearOptions();
        for (const FString& ModeOption : ViewModel->WindowModeOptions)
        {
            Cmb_WindowMode->AddOption(ModeOption);
        }
        if (ViewModel->WindowModeOptions.IsValidIndex(ViewModel->WindowMode))
        {
            Cmb_WindowMode->SetSelectedOption(ViewModel->WindowModeOptions[ViewModel->WindowMode]);
        }
        OriginalWindowMode = ViewModel->WindowMode;
    }

    // Update VSync checkbox
    if (Chk_VSync)
    {
        Chk_VSync->SetIsChecked(ViewModel->bUseVSync);
        bOriginalVSync = ViewModel->bUseVSync;
    }

    // Update Shadow Quality combo box
    if (Cmb_ShadowQuality)
    {
        Cmb_ShadowQuality->ClearOptions();
        for (const FString& QualityOption : ViewModel->QualityOptions)
        {
            Cmb_ShadowQuality->AddOption(QualityOption);
        }
        if (ViewModel->QualityOptions.IsValidIndex(ViewModel->ShadowQuality))
        {
            Cmb_ShadowQuality->SetSelectedOption(ViewModel->QualityOptions[ViewModel->ShadowQuality]);
        }
        OriginalShadowQuality = ViewModel->ShadowQuality;
    }

    // Update Texture Quality combo box
    if (Cmb_TextureQuality)
    {
        Cmb_TextureQuality->ClearOptions();
        for (const FString& QualityOption : ViewModel->QualityOptions)
        {
            Cmb_TextureQuality->AddOption(QualityOption);
        }
        if (ViewModel->QualityOptions.IsValidIndex(ViewModel->TextureQuality))
        {
            Cmb_TextureQuality->SetSelectedOption(ViewModel->QualityOptions[ViewModel->TextureQuality]);
        }
        OriginalTextureQuality = ViewModel->TextureQuality;
    }

    // Update Anti-Aliasing combo box
    if (Cmb_AntiAliasing)
    {
        Cmb_AntiAliasing->ClearOptions();
        for (const FString& AAOption : ViewModel->AntiAliasingOptions)
        {
            Cmb_AntiAliasing->AddOption(AAOption);
        }
        if (ViewModel->AntiAliasingOptions.IsValidIndex(ViewModel->AntiAliasingMode))
        {
            Cmb_AntiAliasing->SetSelectedOption(ViewModel->AntiAliasingOptions[ViewModel->AntiAliasingMode]);
        }
        OriginalAntiAliasingMode = ViewModel->AntiAliasingMode;
    }
}

void US_UI_VideoSettingsTab::OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (ViewModel.IsValid() && SelectionType != ESelectInfo::Direct)
    {
        int32 NewIndex = ViewModel->ResolutionOptions.Find(SelectedItem);
        if (NewIndex != INDEX_NONE)
        {
            ViewModel->ResolutionIndex = NewIndex;
        }
    }
}

void US_UI_VideoSettingsTab::OnWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (ViewModel.IsValid() && SelectionType != ESelectInfo::Direct)
    {
        int32 NewIndex = ViewModel->WindowModeOptions.Find(SelectedItem);
        if (NewIndex != INDEX_NONE)
        {
            ViewModel->WindowMode = NewIndex;
        }
    }
}

void US_UI_VideoSettingsTab::OnVSyncChanged(bool bIsChecked)
{
    if (ViewModel.IsValid())
    {
        ViewModel->bUseVSync = bIsChecked;
    }
}

void US_UI_VideoSettingsTab::OnShadowQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (ViewModel.IsValid() && SelectionType != ESelectInfo::Direct)
    {
        int32 NewIndex = ViewModel->QualityOptions.Find(SelectedItem);
        if (NewIndex != INDEX_NONE)
        {
            ViewModel->ShadowQuality = NewIndex;
        }
    }
}

void US_UI_VideoSettingsTab::OnTextureQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (ViewModel.IsValid() && SelectionType != ESelectInfo::Direct)
    {
        int32 NewIndex = ViewModel->QualityOptions.Find(SelectedItem);
        if (NewIndex != INDEX_NONE)
        {
            ViewModel->TextureQuality = NewIndex;
        }
    }
}

void US_UI_VideoSettingsTab::OnAntiAliasingChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (ViewModel.IsValid() && SelectionType != ESelectInfo::Direct)
    {
        int32 NewIndex = ViewModel->AntiAliasingOptions.Find(SelectedItem);
        if (NewIndex != INDEX_NONE)
        {
            ViewModel->AntiAliasingMode = NewIndex;
        }
    }
}

void US_UI_VideoSettingsTab::ApplySettings()
{
    // Update original values to current values
    if (ViewModel.IsValid())
    {
        OriginalResolutionIndex = ViewModel->ResolutionIndex;
        OriginalWindowMode = ViewModel->WindowMode;
        bOriginalVSync = ViewModel->bUseVSync;
        OriginalShadowQuality = ViewModel->ShadowQuality;
        OriginalTextureQuality = ViewModel->TextureQuality;
        OriginalAntiAliasingMode = ViewModel->AntiAliasingMode;
    }
}

void US_UI_VideoSettingsTab::RevertSettings()
{
    if (ViewModel.IsValid())
    {
        ViewModel->ResolutionIndex = OriginalResolutionIndex;
        ViewModel->WindowMode = OriginalWindowMode;
        ViewModel->bUseVSync = bOriginalVSync;
        ViewModel->ShadowQuality = OriginalShadowQuality;
        ViewModel->TextureQuality = OriginalTextureQuality;
        ViewModel->AntiAliasingMode = OriginalAntiAliasingMode;

        // Update UI
        OnViewModelDataChanged();
    }
}

bool US_UI_VideoSettingsTab::HasUnsavedChanges() const
{
    if (!ViewModel.IsValid())
    {
        return false;
    }

    return ViewModel->ResolutionIndex != OriginalResolutionIndex ||
        ViewModel->WindowMode != OriginalWindowMode ||
        ViewModel->bUseVSync != bOriginalVSync ||
        ViewModel->ShadowQuality != OriginalShadowQuality ||
        ViewModel->TextureQuality != OriginalTextureQuality ||
        ViewModel->AntiAliasingMode != OriginalAntiAliasingMode;
}