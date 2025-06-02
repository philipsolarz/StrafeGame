#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "S_KillfeedItemWidget.generated.h"

class UTextBlock;
class UImage;
class US_KillfeedItemViewModel;

UCLASS()
class STRAFEGAME_API US_KillfeedItemWidget : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "ViewModel")
    TObjectPtr<US_KillfeedItemViewModel> ItemViewModel;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> TxtKillerName;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TObjectPtr<UImage> ImgKillWeaponIcon; // Renamed for clarity

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> TxtVictimName;

    void RefreshDisplay();
};