// S_UI_TextButton.h

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "S_UI_TextButton.generated.h"

/**
 * A reusable button that exposes a Text property to be set on instances.
 */
UCLASS()
class STRAFEUI_API US_UI_TextButton : public UCommonButtonBase
{
    GENERATED_BODY()

public:
    /** Sets the button's text and updates the visual representation. */
    UFUNCTION(BlueprintCallable, Category = "Button Text")
    void SetButtonText(const FText& InText);

    // This is the C++ equivalent of an "Instance Editable" variable.
    // It will show up in the Details panel of any WBP_TextButton instance.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Text")
    FText ButtonText;

protected:
    // This function runs in the editor, allowing us to see our changes live.
    virtual void NativePreConstruct() override;

    // This binds the UMG TextBlock widget named "Text_Label" to this C++ pointer.
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_Label;
};