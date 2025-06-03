// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "UI/MainMenu/MenuScreenInterface.h" // Updated include
#include "S_FindGameScreen.generated.h"

class UCommonButtonBase;
class UEditableTextBox;
class US_ServerRowData; // Updated: Now US_ServerRowData for list items
class UListView;
class UMenuManagerSubsystem;

UCLASS()
class STRAFEGAME_API US_FindGameScreen : public UCommonActivatableWidget, public IMenuScreenInterface
{
    GENERATED_BODY()

public:
    US_FindGameScreen();
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // IMenuScreenInterface
    virtual void SetMenuManager_Implementation(UMenuManagerSubsystem* InMenuManager) override;

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UEditableTextBox> Txt_Filter;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UListView> ServerListView;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_RefreshList;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_JoinGame;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Back;

    UFUNCTION()
    void OnRefreshListClicked();

    UFUNCTION()
    void OnJoinGameClicked();

    UFUNCTION()
    void OnBackClicked();

    UFUNCTION()
    void OnFilterTextChanged(const FText& Text);

    UFUNCTION()
    void OnServerSelected(UObject* Item);

    // Online Session
    void FindGameSessions();
    void OnFindSessionsComplete(bool bWasSuccessful);
    FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
    FDelegateHandle FindSessionsCompleteDelegateHandle;
    TSharedPtr<FOnlineSessionSearch> SessionSearch;

    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
    FDelegateHandle JoinSessionCompleteDelegateHandle;

private:
    UPROPERTY()
    TObjectPtr<UMenuManagerSubsystem> MenuManager;

    TOptional<FOnlineSessionSearchResult> SelectedSessionResult;

    void PopulateServerList(const TArray<FOnlineSessionSearchResult>& SearchResults);
};