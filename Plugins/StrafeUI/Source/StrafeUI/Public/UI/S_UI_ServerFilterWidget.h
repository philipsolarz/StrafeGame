// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_ServerFilterWidget.h

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "S_UI_ServerFilterWidget.generated.h"

class UEditableTextBox;
class US_UI_StringComboBox;
class UCheckBox;
class USlider;
class UTextBlock;

/**
 * Delegate broadcast when filter values change
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFiltersChanged);

/**
 * Widget that provides filtering options for the server browser
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_ServerFilterWidget : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    /** Event fired when any filter value changes */
    UPROPERTY(BlueprintAssignable, Category = "Server Filter")
    FOnFiltersChanged OnFiltersChanged;

    /** Gets the current server name filter */
    UFUNCTION(BlueprintCallable, Category = "Server Filter")
    FString GetServerNameFilter() const;

    /** Gets the current game mode filter */
    UFUNCTION(BlueprintCallable, Category = "Server Filter")
    FString GetGameModeFilter() const;

    /** Gets whether to hide full servers */
    UFUNCTION(BlueprintCallable, Category = "Server Filter")
    bool GetHideFullServers() const;

    /** Gets whether to hide empty servers */
    UFUNCTION(BlueprintCallable, Category = "Server Filter")
    bool GetHideEmptyServers() const;

    /** Gets whether to hide private servers */
    UFUNCTION(BlueprintCallable, Category = "Server Filter")
    bool GetHidePrivateServers() const;

    /** Gets the maximum ping filter */
    UFUNCTION(BlueprintCallable, Category = "Server Filter")
    int32 GetMaxPing() const;

    /** Resets all filters to default values */
    UFUNCTION(BlueprintCallable, Category = "Server Filter")
    void ResetFilters();

protected:
    virtual void NativeOnInitialized() override;

private:
    // Filter change handlers
    UFUNCTION()
    void OnServerNameChanged(const FText& Text);

    UFUNCTION()
    void OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnHideFullServersChanged(bool bIsChecked);

    UFUNCTION()
    void OnHideEmptyServersChanged(bool bIsChecked);

    UFUNCTION()
    void OnHidePrivateServersChanged(bool bIsChecked);

    UFUNCTION()
    void OnMaxPingChanged(float Value);

    /** Broadcasts that filters have changed */
    void BroadcastFilterChange();

    // UI Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableTextBox> Txt_ServerName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<US_UI_StringComboBox> Cmb_GameMode;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCheckBox> Chk_HideFullServers;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCheckBox> Chk_HideEmptyServers;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCheckBox> Chk_HidePrivateServers;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> Sld_MaxPing;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_MaxPingValue;
};