#include "UI/S_UI_PauseMenuWidget.h"
#include "CommonButtonBase.h"
#include "S_UI_Subsystem.h"
#include "S_UI_Navigator.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/S_UI_InGameActionsWidget.h"

void US_UI_PauseMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Btn_Resume)
    {
        Btn_Resume->OnClicked().AddUObject(this, &US_UI_PauseMenuWidget::HandleResumeClicked);
    }
    if (Btn_Settings)
    {
        Btn_Settings->OnClicked().AddUObject(this, &US_UI_PauseMenuWidget::HandleSettingsClicked);
    }
    if (Btn_InGameActions)
    {
        Btn_InGameActions->OnClicked().AddUObject(this, &US_UI_PauseMenuWidget::HandleInGameActionsClicked);
    }
    if (Btn_ExitToMenu)
    {
        Btn_ExitToMenu->OnClicked().AddUObject(this, &US_UI_PauseMenuWidget::HandleExitToMenuClicked);
    }
    if (Btn_ExitToDesktop)
    {
        Btn_ExitToDesktop->OnClicked().AddUObject(this, &US_UI_PauseMenuWidget::HandleExitToDesktopClicked);
    }
}

void US_UI_PauseMenuWidget::HandleResumeClicked()
{
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        UISubsystem->TogglePauseMenu();
    }
}

void US_UI_PauseMenuWidget::HandleSettingsClicked()
{
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        UISubsystem->GetNavigator()->SwitchContentScreen(E_UIScreenId::Settings);
    }
}

void US_UI_PauseMenuWidget::HandleInGameActionsClicked()
{
    if (InGameActionsWidgetClass)
    {
        if (!InGameActionsWidgetInstance)
        {
            InGameActionsWidgetInstance = CreateWidget<US_UI_InGameActionsWidget>(this, InGameActionsWidgetClass);
        }

        if (InGameActionsWidgetInstance && !InGameActionsWidgetInstance->IsInViewport())
        {
            InGameActionsWidgetInstance->AddToViewport(1); // Add with a higher Z-order
        }
    }
}

void US_UI_PauseMenuWidget::HandleExitToMenuClicked()
{
    UGameplayStatics::OpenLevel(this, FName(TEXT("/StrafeUI/Maps/MainMenu")));
}

void US_UI_PauseMenuWidget::HandleExitToDesktopClicked()
{
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
}