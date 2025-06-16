// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_KeyBindingWidget.h

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "InputCoreTypes.h"
#include "S_UI_KeyBindingWidget.generated.h"

class UCommonTextBlock;
class UCommonButtonBase;
class UBorder;

/**
 * Delegate fired when a key binding changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKeyBindingChanged, FName, ActionName, FKey, NewKey);

/**
 * Widget for displaying and capturing key bindings
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_KeyBindingWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    /** The name of the input action this binding is for */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Binding")
    FName ActionName;

    /** Display name for the action */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Binding")
    FText ActionDisplayName;

    /** The currently bound key */
    UPROPERTY(BlueprintReadOnly, Category = "Key Binding")
    FKey CurrentKey;

    /** Event fired when the key binding changes */
    UPROPERTY(BlueprintAssignable, Category = "Key Binding")
    FOnKeyBindingChanged OnKeyBindingChanged;

    /** Set the current key binding */
    UFUNCTION(BlueprintCallable, Category = "Key Binding")
    void SetKeyBinding(const FKey& InKey);

    /** Start listening for a new key binding */
    UFUNCTION(BlueprintCallable, Category = "Key Binding")
    void StartKeyCapture();

    /** Cancel key capture mode */
    UFUNCTION(BlueprintCallable, Category = "Key Binding")
    void CancelKeyCapture();

    /** Check if currently capturing a key */
    UFUNCTION(BlueprintCallable, Category = "Key Binding")
    bool IsCapturingKey() const { return bIsCapturingKey; }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
    UFUNCTION()
    void HandleBindButtonClicked();

    void UpdateDisplay();
    void FinishKeyCapture(const FKey& InKey);

    // UI Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_ActionName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_CurrentKey;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Bind;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> Border_Background;

    bool bIsCapturingKey = false;
};

