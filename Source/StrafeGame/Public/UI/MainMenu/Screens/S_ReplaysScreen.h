// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "UI/MainMenu/MenuScreenInterface.h" 
#include "S_ReplaysScreen.generated.h"

class UCommonButtonBase;
class UListView;
class UMenuManagerSubsystem;

UCLASS(BlueprintType)
class STRAFEGAME_API US_ReplayListItemData : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly, Category = "Replay Data") FString FileName;
    UPROPERTY(BlueprintReadOnly, Category = "Replay Data") FDateTime Timestamp;
    UPROPERTY(BlueprintReadOnly, Category = "Replay Data") FTimespan Duration;
};


UCLASS()
class STRAFEGAME_API US_ReplaysScreen : public UCommonActivatableWidget, public IMenuScreenInterface
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    virtual void SetMenuManager_Implementation(UMenuManagerSubsystem* InMenuManager) override;

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UListView> ReplayListView;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_PlayReplay;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_DeleteReplay;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Back;

    UFUNCTION()
    void OnPlayReplayClicked();

    UFUNCTION()
    void OnDeleteReplayClicked();

    UFUNCTION()
    void OnBackClicked();

    UFUNCTION()
    void OnReplaySelected(UObject* Item);

private:
    UPROPERTY()
    TObjectPtr<UMenuManagerSubsystem> MenuManager;

    UPROPERTY()
    TObjectPtr<US_ReplayListItemData> SelectedReplayData;

    void PopulateReplayList();

    // Added declaration for the handler
    void HandleDeleteReplayConfirmed(bool bConfirmed);
};