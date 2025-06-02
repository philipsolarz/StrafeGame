#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h" // Required for ListView items
#include "S_WeaponInventoryListItemWidget.generated.h"

class UTextBlock;
class UImage;
class UBorder;
class US_WeaponViewModel;

UCLASS()
class STRAFEGAME_API US_WeaponInventoryListItemWidget : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    /** Called when this item is set with its data object from the ListView */
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

protected:
    // This is the ViewModel for this specific item, set by NativeOnListItemObjectSet
    UPROPERTY(BlueprintReadOnly, Category = "ViewModel")
    TObjectPtr<US_WeaponViewModel> ItemViewModel;

    // Bind these UPROPERTYs to UMG elements in your WBP_WeaponInventory_Item Blueprint
    // Ensure the UMG element names in the WBP match these variable names.
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> TxtWeaponName;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> TxtAmmoCount;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> ImgWeaponIcon;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UBorder> HighlightIndicator; // Example for showing if equipped

    /** Called by the ViewModel when its data changes, or after ItemViewModel is set */
    UFUNCTION()
    virtual void RefreshItemDisplay();
};