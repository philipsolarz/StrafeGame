#include "S_UI_Navigator.h"
#include "S_UI_AssetManager.h"
#include "UI/S_UI_RootWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "ViewModel/S_UI_ViewModelBase.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "UI/S_UI_FindGameWidget.h"
#include "UI/S_UI_SettingsWidget.h"
#include "UI/S_UI_CreateGameWidget.h"

void US_UI_Navigator::Initialize(US_UI_RootWidget* InRootWidget, US_UI_AssetManager* InAssetManager)
{
    UIRootWidget = InRootWidget;
    AssetManager = InAssetManager;

    // If a screen switch was requested while assets were loading, execute it now.
    if (PendingScreenRequest != E_UIScreenId::None)
    {
        UE_LOG(LogTemp, Log, TEXT("Navigator: Processing pending screen request: %s"), *UEnum::GetValueAsString(PendingScreenRequest));
        SwitchContentScreen(PendingScreenRequest);
        PendingScreenRequest = E_UIScreenId::None;
    }
}

void US_UI_Navigator::SwitchContentScreen(E_UIScreenId ScreenId)
{
    if (!AssetManager.IsValid() || !UIRootWidget.IsValid()) return;

    if (!AssetManager->AreAssetsLoaded())
    {
        UE_LOG(LogTemp, Warning, TEXT("Navigator: SwitchContentScreen called for %s while assets are still loading. Request is queued."), *UEnum::GetValueAsString(ScreenId));
        PendingScreenRequest = ScreenId;
        return;
    }

    if (ScreenId == E_UIScreenId::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("Navigator: SwitchContentScreen failed: Invalid ScreenId 'None' provided."));
        return;
    }

    if (!UIRootWidget->GetContentStack())
    {
        UE_LOG(LogTemp, Error, TEXT("Navigator: SwitchContentScreen failed: ContentStack is null."));
        return;
    }

    UIRootWidget->GetContentStack()->ClearWidgets();

    if (TSubclassOf<UCommonActivatableWidget> FoundWidgetClass = AssetManager->GetScreenWidgetClass(ScreenId))
    {
        UCommonActivatableWidget* PushedWidget = UIRootWidget->GetContentStack()->AddWidget<UCommonActivatableWidget>(FoundWidgetClass);
        UE_LOG(LogTemp, Verbose, TEXT("Navigator: Switched content screen to: %s"), *UEnum::GetValueAsString(ScreenId));

        if (IViewModelProvider* ViewModelProvider = Cast<IViewModelProvider>(PushedWidget))
        {
            US_UI_ViewModelBase* ViewModel = ViewModelProvider->CreateViewModel();
            if (US_UI_BaseScreenWidget* BaseScreenWidget = Cast<US_UI_BaseScreenWidget>(PushedWidget))
            {
                // This logic is now properly contained within the navigator
                if (US_UI_FindGameWidget* FindGameWidget = Cast<US_UI_FindGameWidget>(BaseScreenWidget))
                {
                    FindGameWidget->SetViewModel(ViewModel);
                }
                else if (US_UI_SettingsWidget* SettingsWidget = Cast<US_UI_SettingsWidget>(BaseScreenWidget))
                {
                    SettingsWidget->SetViewModel(ViewModel);
                }
                else if (US_UI_CreateGameWidget* CreateGameWidget = Cast<US_UI_CreateGameWidget>(BaseScreenWidget))
                {
                    CreateGameWidget->SetViewModel(ViewModel);
                }
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Navigator: SwitchContentScreen failed: No widget class found for ScreenId %s."), *UEnum::GetValueAsString(ScreenId));
    }
}

void US_UI_Navigator::PopContentScreen()
{
    if (UIRootWidget.IsValid() && UIRootWidget->GetContentStack())
    {
        if (UCommonActivatableWidget* ActiveWidget = UIRootWidget->GetContentStack()->GetActiveWidget())
        {
            ActiveWidget->DeactivateWidget();
            UE_LOG(LogTemp, Verbose, TEXT("Navigator: Popping current content screen."));
        }
    }
}