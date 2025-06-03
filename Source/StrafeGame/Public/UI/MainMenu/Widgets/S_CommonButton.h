// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "S_CommonButton.generated.h"

/**
 * 
 */
UCLASS()
class STRAFEGAME_API US_CommonButton : public UCommonButtonBase
{
	GENERATED_BODY()
public:
    US_CommonButton();

    // Optional: Expose a UPROPERTY for the TextBlock if you want to set text from C++
    UPROPERTY(BlueprintReadWrite, Category = "Button Text", meta = (BindWidgetOptional))
    TObjectPtr<class UTextBlock> ButtonTextBlock;

    UFUNCTION(BlueprintCallable, Category = "Button Text")
    void SetButtonText(const FText& InText);
};
