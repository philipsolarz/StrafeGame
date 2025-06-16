// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_TabControl.cpp

#include "UI/S_UI_TabControl.h"
#include "UI/S_UI_TabButton.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "S_UI_Settings.h"
#include "CommonTabListWidgetBase.h"
#include "CommonActivatableWidgetSwitcher.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"

void US_UI_TabControl::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (TabList && ContentSwitcher)
    {
        TabList->OnTabSelected.AddDynamic(this, &US_UI_TabControl::HandleTabSelected);
        // We no longer rely on this delegate for setting text, but it's good practice to keep the binding.
        TabList->OnTabButtonCreation.AddDynamic(this, &US_UI_TabControl::HandleTabButtonCreation);
        TabList->SetLinkedSwitcher(ContentSwitcher);
    }
}

void US_UI_TabControl::NativeConstruct()
{
    Super::NativeConstruct();

    if (TabList)
    {
        TabList->ForceLayoutPrepass();

        if (TabList->GetVisibility() != ESlateVisibility::Visible)
        {
            TabList->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void US_UI_TabControl::NativeDestruct()
{
    if (TabList)
    {
        TabList->OnTabSelected.RemoveDynamic(this, &US_UI_TabControl::HandleTabSelected);
        TabList->OnTabButtonCreation.RemoveDynamic(this, &US_UI_TabControl::HandleTabButtonCreation);
    }
    CancelAsyncLoad();
    Super::NativeDestruct();
}

void US_UI_TabControl::CancelAsyncLoad()
{
    if (AllAssetsHandle.IsValid())
    {
        AllAssetsHandle->CancelHandle();
        AllAssetsHandle.Reset();
    }
}

void US_UI_TabControl::InitializeTabs(const TArray<FTabDefinition>& InTabDefinitions, int32 DefaultTabIndex)
{
    CancelAsyncLoad();

    if (!TabList || !ContentSwitcher)
    {
        UE_LOG(LogTemp, Error, TEXT("TabControl: TabList or ContentSwitcher is null!"));
        return;
    }

    const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
    if (!Settings || Settings->TabButtonClass.IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("TabControl: No tab button class specified in settings!"));
        return;
    }

    TabDefinitions = InTabDefinitions;
    PendingDefaultTabIndex = DefaultTabIndex;

    TArray<FSoftObjectPath> AssetsToLoad;
    AssetsToLoad.Add(Settings->TabButtonClass.ToSoftObjectPath());
    for (const FTabDefinition& TabDef : TabDefinitions)
    {
        if (!TabDef.ContentWidgetClass.IsNull())
        {
            AssetsToLoad.Add(TabDef.ContentWidgetClass.ToSoftObjectPath());
        }
    }

    FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
    AllAssetsHandle = StreamableManager.RequestAsyncLoad(AssetsToLoad, [WeakThis = TWeakObjectPtr<US_UI_TabControl>(this)]()
        {
            if (WeakThis.IsValid())
            {
                WeakThis->OnAllTabAssetsLoaded();
            }
        });
}

void US_UI_TabControl::OnAllTabAssetsLoaded()
{
    const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
    TSubclassOf<UCommonButtonBase> TabButtonClass = Settings ? Settings->TabButtonClass.Get() : nullptr;

    if (!TabButtonClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TabControl: Failed to load TabButtonClass!"));
        return;
    }

    TabList->RemoveAllTabs();
    ContentSwitcher->ClearChildren();
    TabIndexMap.Empty();
    TabIdCounter = 0;

    // First, register all tabs. This populates the TabList with new or recycled buttons.
    for (int32 i = 0; i < TabDefinitions.Num(); ++i)
    {
        const FTabDefinition& TabDef = TabDefinitions[i];
        FName TabId = FName(*FString::Printf(TEXT("Tab_%d"), TabIdCounter++));
        TabIndexMap.Add(TabId, i);

        UCommonActivatableWidget* ContentWidget = nullptr;
        if (UClass* WidgetClass = TabDef.ContentWidgetClass.Get())
        {
            ContentWidget = CreateWidget<UCommonActivatableWidget>(this, WidgetClass);
            if (ContentWidget)
            {
                ContentSwitcher->AddChild(ContentWidget);
            }
        }
        TabList->RegisterTab(TabId, TabButtonClass, ContentWidget);
    }

    // After registering all tabs, iterate through the newly created tab buttons to set their display text.
    // This is done here to ensure it works correctly with CommonUI's widget recycling/pooling system,
    // which can make the OnTabButtonCreation delegate unreliable for this purpose.
    for (const TPair<FName, int32>& TabPair : TabIndexMap)
    {
        const FName& TabId = TabPair.Key;
        const int32& TabIndex = TabPair.Value;

        if (US_UI_TabButton* CustomTabButton = Cast<US_UI_TabButton>(TabList->GetTabButtonBaseByID(TabId)))
        {
            if (TabDefinitions.IsValidIndex(TabIndex))
            {
                CustomTabButton->SetTabLabelText(TabDefinitions[TabIndex].TabName);
            }
        }
    }

    if (TabDefinitions.IsValidIndex(PendingDefaultTabIndex))
    {
        SelectTabByIndex(PendingDefaultTabIndex);
    }
    else if (TabList->GetTabCount() > 0)
    {
        SelectTabByIndex(0);
    }

    OnTabsInitialized.Broadcast();
}

void US_UI_TabControl::SelectTabByIndex(int32 TabIndex)
{
    if (!TabList || !TabDefinitions.IsValidIndex(TabIndex))
    {
        return;
    }

    for (const auto& Pair : TabIndexMap)
    {
        if (Pair.Value == TabIndex)
        {
            if (TabList->GetSelectedTabId() != Pair.Key)
            {
                TabList->SelectTabByID(Pair.Key);
            }
            break;
        }
    }
}

void US_UI_TabControl::SelectTabByTag(FName TabTag)
{
    for (int32 i = 0; i < TabDefinitions.Num(); ++i)
    {
        if (TabDefinitions[i].TabTag == TabTag)
        {
            SelectTabByIndex(i);
            break;
        }
    }
}

UCommonActivatableWidget* US_UI_TabControl::GetActiveTabContent() const
{
    if (!ContentSwitcher)
    {
        return nullptr;
    }
    return Cast<UCommonActivatableWidget>(ContentSwitcher->GetActiveWidget());
}

int32 US_UI_TabControl::GetSelectedTabIndex() const
{
    if (!TabList)
    {
        return -1;
    }

    FName SelectedTabId = TabList->GetSelectedTabId();
    if (const int32* Index = TabIndexMap.Find(SelectedTabId))
    {
        return *Index;
    }
    return -1;
}

void US_UI_TabControl::HandleTabSelected(FName TabId)
{
    if (const int32* Index = TabIndexMap.Find(TabId))
    {
        if (ContentSwitcher && ContentSwitcher->GetActiveWidgetIndex() != *Index)
        {
            ContentSwitcher->SetActiveWidgetIndex(*Index);
        }

        FName TabTag = (TabDefinitions.IsValidIndex(*Index)) ? TabDefinitions[*Index].TabTag : NAME_None;
        OnTabSelected.Broadcast(*Index, TabTag);
    }
}

void US_UI_TabControl::HandleTabButtonCreation(FName TabId, UCommonButtonBase* TabButton)
{
    // We no longer set text here, but ensuring visibility is still good practice.
    if (TabButton)
    {
        TabButton->SetVisibility(ESlateVisibility::Visible);
    }
}