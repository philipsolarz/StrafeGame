// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_ControlsSettingsTab.cpp

#include "UI/S_UI_ControlsSettingsTab.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/Border.h"
#include "CommonButtonBase.h"
#include "UI/S_UI_KeyBindingWidget.h"
#include "ViewModel/S_UI_VM_Settings.h"
#include "System/S_GameUserSettings.h"
#include "Styling/CoreStyle.h"

void US_UI_ControlsSettingsTab::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Bind UI events
    if (Slider_MouseSensitivity)
    {
        Slider_MouseSensitivity->OnValueChanged.AddDynamic(this, &US_UI_ControlsSettingsTab::OnMouseSensitivityChanged);
        Slider_MouseSensitivity->SetMinValue(0.1f);
        Slider_MouseSensitivity->SetMaxValue(3.0f);
        Slider_MouseSensitivity->SetStepSize(0.1f);
    }

    if (Chk_InvertY)
    {
        Chk_InvertY->OnCheckStateChanged.AddDynamic(this, &US_UI_ControlsSettingsTab::OnInvertYChanged);
    }

    if (Btn_ResetToDefaults)
    {
        Btn_ResetToDefaults->OnClicked().AddUObject(this, &US_UI_ControlsSettingsTab::OnResetToDefaultsClicked);
    }
}

void US_UI_ControlsSettingsTab::NativeConstruct()
{
    Super::NativeConstruct();

    // Populate key binding widgets if they haven't been already
    if (ViewModel.IsValid() && KeyBindingWidgets.Num() == 0)
    {
        PopulateKeyBindings();
    }
}

void US_UI_ControlsSettingsTab::OnViewModelDataChanged()
{
    if (!ViewModel.IsValid())
    {
        return;
    }

    // Update Mouse Sensitivity
    if (Slider_MouseSensitivity)
    {
        Slider_MouseSensitivity->SetValue(ViewModel->MouseSensitivity);
        OriginalMouseSensitivity = ViewModel->MouseSensitivity;
    }

    if (Text_MouseSensitivityValue)
    {
        FNumberFormattingOptions Opts;
        Opts.SetMaximumFractionalDigits(1);
        Text_MouseSensitivityValue->SetText(FText::AsNumber(ViewModel->MouseSensitivity, &Opts));
    }

    // Update Invert Y
    if (Chk_InvertY)
    {
        Chk_InvertY->SetIsChecked(ViewModel->bInvertYAxis);
        bOriginalInvertY = ViewModel->bInvertYAxis;
    }

    // Update Keybindings from ViewModel
    InputBindings = ViewModel->KeyBindings;
    OriginalInputBindings = ViewModel->KeyBindings; // Cache for revert check
    PopulateKeyBindings();
}

void US_UI_ControlsSettingsTab::PopulateKeyBindings()
{
    if (!ScrollBox_KeyBindings || !KeyBindingWidgetClass)
    {
        return;
    }

    // Clear existing widgets
    ScrollBox_KeyBindings->ClearChildren();
    KeyBindingWidgets.Empty();

    FString LastCategory;

    for (const FStrafeInputActionBinding& Binding : InputBindings)
    {
        // Add category header if changed
        if (Binding.Category != LastCategory)
        {
            LastCategory = Binding.Category;

            UBorder* CategoryContainer = NewObject<UBorder>(this);
            if (CategoryContainer)
            {
                CategoryContainer->SetPadding(FMargin(4, 10, 4, 2));
                CategoryContainer->SetBrushColor(FLinearColor::Transparent);

                UTextBlock* CategoryHeader = NewObject<UTextBlock>(this);
                if (CategoryHeader)
                {
                    CategoryHeader->SetText(FText::FromString(Binding.Category));
                    // Use a predefined engine style to avoid deprecated constructor warnings
                    const FName StyleName = FName(TEXT("NormalText"));
                    FSlateFontInfo FontInfo = FCoreStyle::Get().GetFontStyle(StyleName);
                    FontInfo.Size = 16; // Adjust size as needed
                    CategoryHeader->SetFont(FontInfo);

                    CategoryContainer->AddChild(CategoryHeader);
                }

                ScrollBox_KeyBindings->AddChild(CategoryContainer);
            }
        }

        // Create primary key binding widget
        CreateKeyBindingWidget(Binding);
    }
}

void US_UI_ControlsSettingsTab::CreateKeyBindingWidget(const FStrafeInputActionBinding& Binding)
{
    if (!KeyBindingWidgetClass || !ScrollBox_KeyBindings)
    {
        return;
    }

    US_UI_KeyBindingWidget* KeyBindWidget = CreateWidget<US_UI_KeyBindingWidget>(this, KeyBindingWidgetClass);
    if (KeyBindWidget)
    {
        KeyBindWidget->ActionName = Binding.ActionName;
        KeyBindWidget->ActionDisplayName = Binding.DisplayName;
        KeyBindWidget->SetKeyBinding(Binding.PrimaryKey);
        KeyBindWidget->OnKeyBindingChanged.AddDynamic(this, &US_UI_ControlsSettingsTab::OnKeyBindingChanged);

        ScrollBox_KeyBindings->AddChild(KeyBindWidget);
        KeyBindingWidgets.Add(KeyBindWidget);
    }
}

void US_UI_ControlsSettingsTab::OnMouseSensitivityChanged(float Value)
{
    if (ViewModel.IsValid())
    {
        ViewModel->MouseSensitivity = Value;

        if (Text_MouseSensitivityValue)
        {
            FNumberFormattingOptions Opts;
            Opts.SetMaximumFractionalDigits(1);
            Text_MouseSensitivityValue->SetText(FText::AsNumber(Value, &Opts));
        }
    }
}

void US_UI_ControlsSettingsTab::OnInvertYChanged(bool bIsChecked)
{
    if (ViewModel.IsValid())
    {
        ViewModel->bInvertYAxis = bIsChecked;
    }
}

void US_UI_ControlsSettingsTab::OnKeyBindingChanged(FName ActionName, FKey NewKey)
{
    bool bChanged = false;
    for (FStrafeInputActionBinding& Binding : InputBindings)
    {
        if (Binding.ActionName == ActionName)
        {
            if (Binding.PrimaryKey != NewKey)
            {
                Binding.PrimaryKey = NewKey;
                bChanged = true;
            }
            break;
        }
    }

    if (bChanged && ViewModel.IsValid())
    {
        ViewModel->KeyBindings = InputBindings;
    }
}

void US_UI_ControlsSettingsTab::OnResetToDefaultsClicked()
{
    if (ViewModel.IsValid())
    {
        TArray<FStrafeInputActionBinding> DefaultBindings;
        US_GameUserSettings::GetDefaultActionMappings(DefaultBindings);

        ViewModel->MouseSensitivity = 1.0f;
        ViewModel->bInvertYAxis = false;
        ViewModel->KeyBindings = DefaultBindings;

        ViewModel->BroadcastDataChanged();
    }
}

void US_UI_ControlsSettingsTab::ApplySettings()
{
    if (ViewModel.IsValid())
    {
        ViewModel->KeyBindings = InputBindings;
        OriginalInputBindings = InputBindings;
        OriginalMouseSensitivity = ViewModel->MouseSensitivity;
        bOriginalInvertY = ViewModel->bInvertYAxis;
    }
}

void US_UI_ControlsSettingsTab::RevertSettings()
{
    if (ViewModel.IsValid())
    {
        ViewModel->BroadcastDataChanged();
    }
}

bool US_UI_ControlsSettingsTab::HasUnsavedChanges() const
{
    if (!ViewModel.IsValid())
    {
        return false;
    }

    if (FMath::Abs(ViewModel->MouseSensitivity - OriginalMouseSensitivity) > KINDA_SMALL_NUMBER ||
        ViewModel->bInvertYAxis != bOriginalInvertY)
    {
        return true;
    }

    if (InputBindings.Num() != OriginalInputBindings.Num())
    {
        return true;
    }

    for (int32 i = 0; i < InputBindings.Num(); ++i)
    {
        if (InputBindings[i].PrimaryKey != OriginalInputBindings[i].PrimaryKey ||
            InputBindings[i].SecondaryKey != OriginalInputBindings[i].SecondaryKey)
        {
            return true;
        }
    }

    return false;
}