// Philip Solarz All Rights Reserved


#include "UI/Frontend/Widgets/Widget_SettingsScreen.h"
#include "Input/CommonUIInputTypes.h"
#include "ICommonInputModule.h"

#include "UI/Frontend/FrontendDebugHelper.h"

void UWidget_SettingsScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!ResetAction.IsNull())
	{
		ResetActionHandle = RegisterUIActionBinding(
			FBindUIActionArgs(
				ResetAction,
				true,
				FSimpleDelegate::CreateUObject(this, &ThisClass::OnResetBoundActionTriggered)
			)
		);
	}

	RegisterUIActionBinding(
		FBindUIActionArgs(
			ICommonInputModule::GetSettings().GetDefaultBackAction(),
			true,
			FSimpleDelegate::CreateUObject(this, &ThisClass::OnBackBoundActionTriggered)
		)
	);

}

void UWidget_SettingsScreen::OnResetBoundActionTriggered()
{
	Debug::Print(TEXT("Reset bound action triggered"));
}

void UWidget_SettingsScreen::OnBackBoundActionTriggered()
{
	DeactivateWidget();
}
