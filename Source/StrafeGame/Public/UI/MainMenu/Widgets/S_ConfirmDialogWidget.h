// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "S_ConfirmDialogWidget.generated.h"

class UCommonButtonBase;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConfirmDialogClosedNative, bool, bConfirmed);

UCLASS()
class STRAFEGAME_API US_ConfirmDialogWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Confirm Dialog")
    void SetupDialog(const FText& Title, const FText& Message);

    FOnConfirmDialogClosedNative OnDialogClosedDelegate;

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_Title;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_Message;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Yes;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_No;

    UFUNCTION()
    void OnYesClicked();

    UFUNCTION()
    void OnNoClicked();
};