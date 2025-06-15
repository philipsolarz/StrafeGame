// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_SettingsWidget.cpp

#include "UI/S_UI_SettingsWidget.h"
#include "UI/S_UI_TabControl.h"
#include "UI/S_UI_SettingsTabBase.h"
#include "CommonButtonBase.h"
#include "S_UI_Subsystem.h"
#include "S_UI_Navigator.h"
#include "S_UI_Settings.h"
#include "ViewModel/S_UI_VM_Settings.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

US_UI_ViewModelBase* US_UI_SettingsWidget::CreateViewModel()
{
    US_UI_VM_Settings* VM = NewObject<US_UI_VM_Settings>(this);
    VM->Initialize();
    return VM;
}

void US_UI_SettingsWidget::SetViewModel(US_UI_ViewModelBase* InViewModel)
{
    if (US_UI_VM_Settings* InSettingsViewModel = Cast<US_UI_VM_Settings>(InViewModel))
    {
        ViewModel = InSettingsViewModel;

        // Initialize tabs now that we have the view model, but only do it once.
        if (!bTabsInitialized)
        {
            InitializeSettingsTabs();
        }
    }
}

void US_UI_SettingsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Btn_Apply)
    {
        Btn_Apply->OnClicked().AddUObject(this, &US_UI_SettingsWidget::HandleApplyClicked);
    }
    if (Btn_Revert)
    {
        Btn_Revert->OnClicked().AddUObject(this, &US_UI_SettingsWidget::HandleRevertClicked);
    }
    if (Btn_Back)
    {
        Btn_Back->OnClicked().AddUObject(this, &US_UI_SettingsWidget::HandleBackClicked);
    }
    if (TabControl)
    {
        TabControl->OnTabSelected.AddDynamic(this, &US_UI_SettingsWidget::OnSettingsTabSelected);
    }
}

void US_UI_SettingsWidget::NativeDestruct()
{
    if (TabControl)
    {
        TabControl->CancelAsyncLoad();
        TabControl->OnTabSelected.RemoveDynamic(this, &US_UI_SettingsWidget::OnSettingsTabSelected);
        TabControl->OnTabsInitialized.RemoveDynamic(this, &US_UI_SettingsWidget::HandleTabsInitialized);
    }

    bTabsInitialized = false;

    Super::NativeDestruct();
}

void US_UI_SettingsWidget::InitializeSettingsTabs()
{
    // Prevent re-initialization
    if (bTabsInitialized || !TabControl || !ViewModel.IsValid())
    {
        return;
    }
    bTabsInitialized = true;

    const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
    if (!Settings)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsWidget: No settings found"));
        return;
    }

    TArray<FTabDefinition> TabDefs;
    if (!Settings->AudioSettingsTabClass.IsNull())
    {
        FTabDefinition AudioTab;
        AudioTab.TabName = FText::FromString(TEXT("Audio"));
        AudioTab.ContentWidgetClass = Settings->AudioSettingsTabClass;
        AudioTab.TabTag = "Audio";
        TabDefs.Add(AudioTab);
    }
    if (!Settings->VideoSettingsTabClass.IsNull())
    {
        FTabDefinition VideoTab;
        VideoTab.TabName = FText::FromString(TEXT("Video"));
        VideoTab.ContentWidgetClass = Settings->VideoSettingsTabClass;
        VideoTab.TabTag = "Video";
        TabDefs.Add(VideoTab);
    }
    if (!Settings->ControlsSettingsTabClass.IsNull())
    {
        FTabDefinition ControlsTab;
        ControlsTab.TabName = FText::FromString(TEXT("Controls"));
        ControlsTab.ContentWidgetClass = Settings->ControlsSettingsTabClass;
        ControlsTab.TabTag = "Controls";
        TabDefs.Add(ControlsTab);
    }
    if (!Settings->GameplaySettingsTabClass.IsNull())
    {
        FTabDefinition GameplayTab;
        GameplayTab.TabName = FText::FromString(TEXT("Gameplay"));
        GameplayTab.ContentWidgetClass = Settings->GameplaySettingsTabClass;
        GameplayTab.TabTag = "Gameplay";
        TabDefs.Add(GameplayTab);
    }
    if (!Settings->PlayerSettingsTabClass.IsNull())
    {
        FTabDefinition PlayerTab;
        PlayerTab.TabName = FText::FromString(TEXT("Player"));
        PlayerTab.ContentWidgetClass = Settings->PlayerSettingsTabClass;
        PlayerTab.TabTag = "Player";
        TabDefs.Add(PlayerTab);
    }

    if (TabDefs.Num() > 0 && TabControl)
    {
        // Bind the callback for when the tab control has finished creating the tab content widgets.
        TabControl->OnTabsInitialized.AddDynamic(this, &US_UI_SettingsWidget::HandleTabsInitialized);
        TabControl->InitializeTabs(TabDefs, 0);
    }
}

void US_UI_SettingsWidget::HandleTabsInitialized()
{
    if (TabControl && TabControl->GetContentSwitcher())
    {
        UCommonActivatableWidgetSwitcher* Switcher = TabControl->GetContentSwitcher();
        SettingsTabs.Empty();
        for (int32 i = 0; i < Switcher->GetNumWidgets(); ++i)
        {
            if (US_UI_SettingsTabBase* TabContent = Cast<US_UI_SettingsTabBase>(Switcher->GetWidgetAtIndex(i)))
            {
                TabContent->SetViewModel(ViewModel.Get());
                SettingsTabs.Add(TabContent);
            }
        }
        UE_LOG(LogTemp, Log, TEXT("SettingsWidget: TabControl is ready. Set ViewModels for %d tabs."), SettingsTabs.Num());
    }
}

void US_UI_SettingsWidget::OnSettingsTabSelected(int32 TabIndex, FName TabTag)
{
    UE_LOG(LogTemp, Verbose, TEXT("Settings tab selected: %d (%s)"), TabIndex, *TabTag.ToString());
}

void US_UI_SettingsWidget::HandleApplyClicked()
{
    for (US_UI_SettingsTabBase* Tab : SettingsTabs)
    {
        if (Tab) Tab->ApplySettings();
    }
    if (ViewModel.IsValid()) ViewModel->ApplySettings();
}

void US_UI_SettingsWidget::HandleRevertClicked()
{
    for (US_UI_SettingsTabBase* Tab : SettingsTabs)
    {
        if (Tab) Tab->RevertSettings();
    }
    if (ViewModel.IsValid()) ViewModel->RevertChanges();
}

void US_UI_SettingsWidget::HandleBackClicked()
{
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        bool bHasUnsavedChanges = false;
        for (US_UI_SettingsTabBase* Tab : SettingsTabs)
        {
            if (Tab && Tab->HasUnsavedChanges())
            {
                bHasUnsavedChanges = true;
                break;
            }
        }

        if (bHasUnsavedChanges)
        {
            F_UIModalPayload Payload;
            Payload.Message = FText::FromString(TEXT("You have unsaved changes. Do you want to save before leaving?"));
            Payload.ModalType = E_UIModalType::YesNo;

            UISubsystem->RequestModal(Payload, FOnModalDismissedSignature::CreateLambda([this, UISubsystem](bool bConfirmed)
                {
                    if (bConfirmed) HandleApplyClicked();
                    UISubsystem->GetNavigator()->PopContentScreen();
                }));
        }
        else
        {
            UISubsystem->GetNavigator()->PopContentScreen();
        }
    }
}