// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_TabControl.h

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CommonTabListWidgetBase.h"
#include "Engine/StreamableManager.h"
#include "CommonActivatableWidgetSwitcher.h"
#include "S_UI_TabControl.generated.h"

USTRUCT(BlueprintType)
struct FTabDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText TabName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftClassPtr<UCommonActivatableWidget> ContentWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName TabTag;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTabsInitialized);

UCLASS(Abstract)
class STRAFEUI_API US_UI_TabControl : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    /** Delegate broadcast when all tab assets have been loaded and their content widgets created. */
    UPROPERTY(BlueprintAssignable, Category = "Tab Control")
    FOnTabsInitialized OnTabsInitialized;

    void InitializeTabs(const TArray<FTabDefinition>& InTabDefinitions, int32 DefaultTabIndex = 0);
    void CancelAsyncLoad();
    void SelectTabByIndex(int32 TabIndex);
    void SelectTabByTag(FName TabTag);
    UCommonActivatableWidget* GetActiveTabContent() const;
    int32 GetSelectedTabIndex() const;
    UCommonActivatableWidgetSwitcher* GetContentSwitcher() const { return ContentSwitcher; }

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabSelected, int32, TabIndex, FName, TabTag);
    UPROPERTY(BlueprintAssignable, Category = "Tab Control")
    FOnTabSelected OnTabSelected;

protected:
    // ... same as before
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonTabListWidgetBase> TabList;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonActivatableWidgetSwitcher> ContentSwitcher;

private:
    UFUNCTION()
    void HandleTabSelected(FName TabId);

    UFUNCTION()
    void HandleTabButtonCreation(FName TabId, UCommonButtonBase* TabButton);

    void OnAllTabAssetsLoaded();

    TArray<FTabDefinition> TabDefinitions;
    TMap<FName, int32> TabIndexMap;
    int32 TabIdCounter = 0;
    TSharedPtr<FStreamableHandle> AllAssetsHandle;
    int32 PendingDefaultTabIndex = 0;
};