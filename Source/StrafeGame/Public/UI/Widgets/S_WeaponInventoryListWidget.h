#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S_WeaponInventoryListWidget.generated.h"

class UListView;
class US_PlayerHUDViewModel;

UCLASS()
class STRAFEGAME_API US_WeaponInventoryListWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Call this to set the ViewModel that this list should observe */
    UFUNCTION(BlueprintCallable, Category = "Weapon Inventory List")
    void SetPlayerHUDViewModel(US_PlayerHUDViewModel* InViewModel);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModel")
    TObjectPtr<US_PlayerHUDViewModel> PlayerHUDViewModel;

    // Bind this to the ListView UMG element in your WBP_WeaponInventoryList Blueprint.
    // Ensure the UMG ListView element is named "InventoryListView".
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UListView> InventoryListView;

    /** Called when the PlayerHUDViewModel signals an update */
    UFUNCTION()
    virtual void HandlePlayerHUDViewModelUpdated();

    /** Clears and repopulates the ListView from the ViewModel */
    virtual void RefreshInventoryList();
};