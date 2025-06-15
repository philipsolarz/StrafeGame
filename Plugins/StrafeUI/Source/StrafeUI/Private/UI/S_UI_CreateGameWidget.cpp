// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_CreateGameWidget.cpp

#include "UI/S_UI_CreateGameWidget.h"
#include "ViewModel/S_UI_VM_CreateGame.h"
#include "Components/EditableTextBox.h"
#include "UI/S_UI_StringComboBox.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "UI/S_UI_TextButton.h"
#include "S_UI_Settings.h"
#include "UI/S_UI_CollapsibleBox.h"
#include "Groups/CommonButtonGroupBase.h"

US_UI_ViewModelBase* US_UI_CreateGameWidget::CreateViewModel()
{
    US_UI_VM_CreateGame* VM = NewObject<US_UI_VM_CreateGame>(this);
    VM->Initialize(GetDefault<US_UI_Settings>());
    return VM;
}


void US_UI_CreateGameWidget::SetViewModel(US_UI_ViewModelBase* InViewModel)
{
    if (US_UI_VM_CreateGame* InCreateGameViewModel = Cast<US_UI_VM_CreateGame>(InViewModel))
    {
        if (InCreateGameViewModel)
        {
            ViewModel = InCreateGameViewModel;

            // Now that the viewmodel is set, bind to data changes for dynamic updates.
            ViewModel->OnDataChanged.AddUniqueDynamic(this, &US_UI_CreateGameWidget::OnViewModelDataChanged);

            // Trigger the first update.
            OnViewModelDataChanged();
        }
    }
}

void US_UI_CreateGameWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    GameModeButtonGroup = NewObject<UCommonButtonGroupBase>(this);

    if (Btn_CreateGame)
    {
        Btn_CreateGame->OnClicked().AddUObject(this, &US_UI_CreateGameWidget::OnCreateGameClicked);
    }

    if (Sld_MaxPlayers)
    {
        Sld_MaxPlayers->OnValueChanged.AddDynamic(this, &US_UI_CreateGameWidget::OnMaxPlayersChanged);
    }

    if (Btn_ModeStrafe)
    {
        GameModeButtonGroup->AddWidget(Btn_ModeStrafe);
    }
    if (Btn_ModeArena)
    {
        GameModeButtonGroup->AddWidget(Btn_ModeArena);
    }
    if (Btn_ModeDuel)
    {
        GameModeButtonGroup->AddWidget(Btn_ModeDuel);
    }

    GameModeButtonGroup->OnSelectedButtonBaseChanged.AddDynamic(this, &US_UI_CreateGameWidget::OnGameModeButtonSelected);

}

void US_UI_CreateGameWidget::OnViewModelDataChanged()
{
    if (!ViewModel.IsValid()) return;

    // Update controls from ViewModel data
    Txt_GameName->SetText(FText::FromString(ViewModel->GameName));
    Chk_IsLAN->SetIsChecked(ViewModel->bIsLANMatch);
    Chk_IsDedicatedServer->SetIsChecked(ViewModel->bIsDedicatedServer);
    Sld_MaxPlayers->SetValue(ViewModel->MaxPlayers);
    OnMaxPlayersChanged(ViewModel->MaxPlayers);
    Txt_Password->SetText(FText::FromString(ViewModel->Password));

    // Repopulate the Maps dropdown, as this list changes based on the Game Mode.
    if (Cmb_Map)
    {
        Cmb_Map->ClearOptions();
        for (const FString& MapName : ViewModel->MapDisplayNames)
        {
            Cmb_Map->AddOption(MapName);
        }

        if (ViewModel->MapDisplayNames.Num() > 0)
        {
            Cmb_Map->SetSelectedOption(ViewModel->SelectedMapName);
        }
    }

    // Update the selected game mode button
    const int32 SelectedIndex = ViewModel->GameModeDisplayNames.Find(ViewModel->SelectedGameModeName);
    UE_LOG(LogTemp, Warning, TEXT("[CreateGameWidget] OnViewModelDataChanged: ViewModel requests selection of index %d ('%s')."), SelectedIndex, *ViewModel->SelectedGameModeName);
    if (GameModeButtonGroup->GetButtonCount() > SelectedIndex)
    {
        GameModeButtonGroup->SelectButtonAtIndex(SelectedIndex);
    }
}

void US_UI_CreateGameWidget::OnGameModeButtonSelected(UCommonButtonBase* SelectedButton, int32 ButtonIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("[CreateGameWidget] OnGameModeButtonSelected: User interaction selected button at index %d."), ButtonIndex);
    if (ViewModel.IsValid() && ViewModel->GameModeDisplayNames.IsValidIndex(ButtonIndex))
    {
        const FString& NewGameMode = ViewModel->GameModeDisplayNames[ButtonIndex];
        if (ViewModel->SelectedGameModeName != NewGameMode)
        {
            ViewModel->OnGameModeChanged(NewGameMode);
        }
    }
}

void US_UI_CreateGameWidget::OnMaxPlayersChanged(float Value)
{
    if (Txt_MaxPlayersValue)
    {
        Txt_MaxPlayersValue->SetText(FText::AsNumber(static_cast<int32>(Value)));
    }
}

void US_UI_CreateGameWidget::OnCreateGameClicked()
{
    if (ViewModel.IsValid())
    {
        // Push UI state to the ViewModel before creating the game
        ViewModel->GameName = Txt_GameName->GetText().ToString();
        ViewModel->bIsLANMatch = Chk_IsLAN->IsChecked();
        ViewModel->bIsDedicatedServer = Chk_IsDedicatedServer->IsChecked();
        ViewModel->MaxPlayers = Sld_MaxPlayers->GetValue();
        ViewModel->Password = Txt_Password->GetText().ToString();
        ViewModel->SelectedMapName = Cmb_Map->GetSelectedOption();

        ViewModel->CreateGame();
    }
}