// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_TabListWidget.h

#pragma once

#include "CoreMinimal.h"
#include "CommonTabListWidgetBase.h"
#include "S_UI_TabListWidget.generated.h"

class UPanelWidget;

/**
 * A concrete implementation of UCommonTabListWidgetBase that provides a visual container for tab buttons.
 * In your UMG Blueprint that inherits from this class, you must add a UPanelWidget (like a Horizontal Box or Vertical Box)
 * and bind it to the ButtonContainer property. This allows for flexible layout and orientation control directly in the editor.
 */
UCLASS(Abstract) // Abstract because it's intended to be subclassed in Blueprint to bind the container.
class STRAFEUI_API US_UI_TabListWidget : public UCommonTabListWidgetBase
{
	GENERATED_BODY()

protected:
	//~ UCommonTabListWidgetBase
	/** Overridden to place the newly created tab button into our visual container. */
	virtual void HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;

	/** Overridden to remove the tab button from our visual container. */
	virtual void HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;
	//~ UCommonTabListWidgetBase

	/** The container widget where tab buttons are visually placed. Must be bound in the UMG editor. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> ButtonContainer;
};