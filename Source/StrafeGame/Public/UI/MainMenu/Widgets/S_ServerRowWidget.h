// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "S_ServerRowWidget.generated.h"

class UTextBlock;

// UObject to hold data for the ListView item
UCLASS(BlueprintType) // Make sure this is BlueprintType so ListView can use it
class STRAFEGAME_API US_ServerRowData : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly, Category = "Server Row Data")
    FString ServerName;
    UPROPERTY(BlueprintReadOnly, Category = "Server Row Data")
    FString MapName;
    UPROPERTY(BlueprintReadOnly, Category = "Server Row Data")
    int32 CurrentPlayers;
    UPROPERTY(BlueprintReadOnly, Category = "Server Row Data")
    int32 MaxPlayers;
    UPROPERTY(BlueprintReadOnly, Category = "Server Row Data")
    int32 Ping;
};


UCLASS()
class STRAFEGAME_API US_ServerRowWidget : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_ServerName;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_MapName;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_PlayerCount;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_Ping;

private:
    UPROPERTY()
    TObjectPtr<US_ServerRowData> ItemData;

    void UpdateDisplay();
};