// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_PlayerSettingsTab.cpp

#include "UI/S_UI_PlayerSettingsTab.h"
#include "Components/EditableTextBox.h"
#include "UI/S_UI_StringComboBox.h"
#include "ViewModel/S_UI_VM_Settings.h"

void US_UI_PlayerSettingsTab::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Populate character model options
    CharacterModelOptions.Empty();
    CharacterModelOptions.Add(TEXT("Default"));
    CharacterModelOptions.Add(TEXT("Soldier"));
    CharacterModelOptions.Add(TEXT("Scout"));
    CharacterModelOptions.Add(TEXT("Engineer"));
    CharacterModelOptions.Add(TEXT("Medic"));

    // Bind UI events
    if (Txt_PlayerName)
    {
        Txt_PlayerName->OnTextChanged.AddDynamic(this, &US_UI_PlayerSettingsTab::OnPlayerNameChanged);
    }

    if (Cmb_CharacterModel)
    {
        Cmb_CharacterModel->OnSelectionChanged.AddDynamic(this, &US_UI_PlayerSettingsTab::OnCharacterModelChanged);

        // Populate character model options
        Cmb_CharacterModel->ClearOptions();
        for (const FString& ModelOption : CharacterModelOptions)
        {
            Cmb_CharacterModel->AddOption(ModelOption);
        }
    }
}

void US_UI_PlayerSettingsTab::OnViewModelDataChanged()
{
    if (!ViewModel.IsValid())
    {
        return;
    }

    // Update Player Name
    if (Txt_PlayerName)
    {
        Txt_PlayerName->SetText(FText::FromString(ViewModel->PlayerName));
        OriginalPlayerName = ViewModel->PlayerName;
    }

    // Update Character Model
    if (Cmb_CharacterModel && CharacterModelOptions.IsValidIndex(ViewModel->SelectedCharacterModel))
    {
        Cmb_CharacterModel->SetSelectedOption(CharacterModelOptions[ViewModel->SelectedCharacterModel]);
        OriginalCharacterModel = ViewModel->SelectedCharacterModel;
    }
}

void US_UI_PlayerSettingsTab::OnPlayerNameChanged(const FText& Text)
{
    if (ViewModel.IsValid())
    {
        ViewModel->PlayerName = Text.ToString();
    }
}

void US_UI_PlayerSettingsTab::OnCharacterModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (ViewModel.IsValid() && SelectionType != ESelectInfo::Direct)
    {
        int32 NewIndex = CharacterModelOptions.Find(SelectedItem);
        if (NewIndex != INDEX_NONE)
        {
            ViewModel->SelectedCharacterModel = NewIndex;
        }
    }
}

void US_UI_PlayerSettingsTab::ApplySettings()
{
    if (ViewModel.IsValid())
    {
        OriginalPlayerName = ViewModel->PlayerName;
        OriginalCharacterModel = ViewModel->SelectedCharacterModel;
    }
}

void US_UI_PlayerSettingsTab::RevertSettings()
{
    if (ViewModel.IsValid())
    {
        ViewModel->PlayerName = OriginalPlayerName;
        ViewModel->SelectedCharacterModel = OriginalCharacterModel;

        // Update UI
        OnViewModelDataChanged();
    }
}

bool US_UI_PlayerSettingsTab::HasUnsavedChanges() const
{
    if (!ViewModel.IsValid())
    {
        return false;
    }

    return ViewModel->PlayerName != OriginalPlayerName ||
        ViewModel->SelectedCharacterModel != OriginalCharacterModel;
}