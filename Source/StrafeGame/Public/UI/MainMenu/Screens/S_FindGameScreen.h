// Source/StrafeGame/Public/UI/MainMenu/Screens/S_FindGameScreen.h
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h" 
#include "UI/MainMenu/MenuScreenInterface.h"
#include "S_FindGameScreen.generated.h"

class UCommonButtonBase;
class UEditableTextBox;
class US_ServerRowData;
class UListView;
class AS_MainMenuPlayerController; // Changed

UCLASS()
class STRAFEGAME_API US_FindGameScreen : public UCommonActivatableWidget, public IMenuScreenInterface
{
    GENERATED_BODY()

public:
    US_FindGameScreen();
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // IMenuScreenInterface
    virtual void SetMainMenuPlayerController_Implementation(AS_MainMenuPlayerController* InPlayerController) override; // Updated

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
    TWeakObjectPtr<AS_MainMenuPlayerController> OwningMainMenuPlayerController; // Changed

    TOptional<FOnlineSessionSearchResult> SelectedSessionResult;

    void PopulateServerList(const TArray<FOnlineSessionSearchResult>& SearchResults);
};