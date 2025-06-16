// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_ServerFilterWidget.cpp

#include "UI/S_UI_ServerFilterWidget.h"
#include "Components/EditableTextBox.h"
#include "UI/S_UI_StringComboBox.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"

void US_UI_ServerFilterWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Bind text change events
    if (Txt_ServerName)
    {
        Txt_ServerName->OnTextChanged.AddDynamic(this, &US_UI_ServerFilterWidget::OnServerNameChanged);
    }

    // Setup game mode combo box
    if (Cmb_GameMode)
    {
        // Add default "All" option
        Cmb_GameMode->AddOption(TEXT("All"));

        // Add common game modes - in a real implementation, these would come from game settings
        Cmb_GameMode->AddOption(TEXT("Deathmatch"));
        Cmb_GameMode->AddOption(TEXT("Team Deathmatch"));
        Cmb_GameMode->AddOption(TEXT("Capture the Flag"));
        Cmb_GameMode->AddOption(TEXT("Domination"));

        Cmb_GameMode->SetSelectedOption(TEXT("All"));
        Cmb_GameMode->OnSelectionChanged.AddDynamic(this, &US_UI_ServerFilterWidget::OnGameModeSelectionChanged);
    }

    // Bind checkbox events
    if (Chk_HideFullServers)
    {
        Chk_HideFullServers->OnCheckStateChanged.AddDynamic(this, &US_UI_ServerFilterWidget::OnHideFullServersChanged);
    }
    if (Chk_HideEmptyServers)
    {
        Chk_HideEmptyServers->OnCheckStateChanged.AddDynamic(this, &US_UI_ServerFilterWidget::OnHideEmptyServersChanged);
    }
    if (Chk_HidePrivateServers)
    {
        Chk_HidePrivateServers->OnCheckStateChanged.AddDynamic(this, &US_UI_ServerFilterWidget::OnHidePrivateServersChanged);
    }

    // Setup ping slider
    if (Sld_MaxPing)
    {
        Sld_MaxPing->SetMinValue(0.0f);
        Sld_MaxPing->SetMaxValue(500.0f);
        Sld_MaxPing->SetValue(999.0f); // Default to max
        Sld_MaxPing->OnValueChanged.AddDynamic(this, &US_UI_ServerFilterWidget::OnMaxPingChanged);

        // Update the display text
        if (Txt_MaxPingValue)
        {
            Txt_MaxPingValue->SetText(FText::FromString(TEXT("999")));
        }
    }
}

FString US_UI_ServerFilterWidget::GetServerNameFilter() const
{
    return Txt_ServerName ? Txt_ServerName->GetText().ToString() : FString();
}

FString US_UI_ServerFilterWidget::GetGameModeFilter() const
{
    if (Cmb_GameMode)
    {
        FString SelectedMode = Cmb_GameMode->GetSelectedOption();
        return (SelectedMode == TEXT("All")) ? FString() : SelectedMode;
    }
    return FString();
}

bool US_UI_ServerFilterWidget::GetHideFullServers() const
{
    return Chk_HideFullServers ? Chk_HideFullServers->IsChecked() : false;
}

bool US_UI_ServerFilterWidget::GetHideEmptyServers() const
{
    return Chk_HideEmptyServers ? Chk_HideEmptyServers->IsChecked() : false;
}

bool US_UI_ServerFilterWidget::GetHidePrivateServers() const
{
    return Chk_HidePrivateServers ? Chk_HidePrivateServers->IsChecked() : false;
}

int32 US_UI_ServerFilterWidget::GetMaxPing() const
{
    if (Sld_MaxPing)
    {
        float Value = Sld_MaxPing->GetValue();
        // If slider is at max, return a very high number to effectively disable the filter
        return (Value >= 500.0f) ? 9999 : FMath::RoundToInt(Value);
    }
    return 9999;
}

void US_UI_ServerFilterWidget::ResetFilters()
{
    if (Txt_ServerName)
    {
        Txt_ServerName->SetText(FText::GetEmpty());
    }

    if (Cmb_GameMode)
    {
        Cmb_GameMode->SetSelectedOption(TEXT("All"));
    }

    if (Chk_HideFullServers)
    {
        Chk_HideFullServers->SetIsChecked(false);
    }

    if (Chk_HideEmptyServers)
    {
        Chk_HideEmptyServers->SetIsChecked(false);
    }

    if (Chk_HidePrivateServers)
    {
        Chk_HidePrivateServers->SetIsChecked(false);
    }

    if (Sld_MaxPing)
    {
        Sld_MaxPing->SetValue(999.0f);
    }

    if (Txt_MaxPingValue)
    {
        Txt_MaxPingValue->SetText(FText::FromString(TEXT("999")));
    }

    BroadcastFilterChange();
}

void US_UI_ServerFilterWidget::OnServerNameChanged(const FText& Text)
{
    BroadcastFilterChange();
}

void US_UI_ServerFilterWidget::OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (SelectionType != ESelectInfo::Direct)
    {
        BroadcastFilterChange();
    }
}

void US_UI_ServerFilterWidget::OnHideFullServersChanged(bool bIsChecked)
{
    BroadcastFilterChange();
}

void US_UI_ServerFilterWidget::OnHideEmptyServersChanged(bool bIsChecked)
{
    BroadcastFilterChange();
}

void US_UI_ServerFilterWidget::OnHidePrivateServersChanged(bool bIsChecked)
{
    BroadcastFilterChange();
}

void US_UI_ServerFilterWidget::OnMaxPingChanged(float Value)
{
    // Update the display text
    if (Txt_MaxPingValue)
    {
        if (Value >= 500.0f)
        {
            Txt_MaxPingValue->SetText(FText::FromString(TEXT("Any")));
        }
        else
        {
            Txt_MaxPingValue->SetText(FText::AsNumber(FMath::RoundToInt(Value)));
        }
    }

    BroadcastFilterChange();
}

void US_UI_ServerFilterWidget::BroadcastFilterChange()
{
    OnFiltersChanged.Broadcast();
}