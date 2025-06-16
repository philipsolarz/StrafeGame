// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_CollapsibleBox.h

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "S_UI_CollapsibleBox.generated.h"

class UButton;
class USizeBox;
class UTextBlock;

/**
 * A simple collapsible box widget.
 * This is a user widget that should contain a HeaderButton and a ContentBox in its hierarchy.
 */
UCLASS()
class STRAFEUI_API US_UI_CollapsibleBox : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** The text to display in the header button. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapsible Box", meta = (ExposeOnSpawn = "true"))
	FText HeaderText;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> HeaderButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USizeBox> ContentBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> HeaderLabel;

private:
	UFUNCTION()
	void OnHeaderClicked();

	void UpdateHeaderText();
};