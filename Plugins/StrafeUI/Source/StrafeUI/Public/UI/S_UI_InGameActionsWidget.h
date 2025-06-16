#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "S_UI_InGameActionsWidget.generated.h"

class UCommonButtonBase;

/**
 * A placeholder widget for future in-game actions like voting or spectating.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_InGameActionsWidget : public UCommonUserWidget
{
    GENERATED_BODY()

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Spectate;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_VoteMap;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_VoteKick;
};