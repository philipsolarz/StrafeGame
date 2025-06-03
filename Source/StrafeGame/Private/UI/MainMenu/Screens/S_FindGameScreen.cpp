// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/MainMenu/Screens/S_FindGameScreen.h" // Updated include
#include "CommonButtonBase.h"
#include "Components/EditableTextBox.h"
#include "Components/ListView.h"
#include "UI/MainMenu/Widgets/US_ServerRowWidget.h" // This is the entry widget, ensure US_ServerRowData is also known
#include "UI/MainMenu/MenuManagerSubsystem.h" // Updated include
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

US_FindGameScreen::US_FindGameScreen()
{
}

void US_FindGameScreen::NativeConstruct()
{
    Super::NativeConstruct();

    if (Btn_RefreshList)
    {
        Btn_RefreshList->OnClicked().AddUObject(this, &US_FindGameScreen::OnRefreshListClicked);
    }
    if (Btn_JoinGame)
    {
        Btn_JoinGame->OnClicked().AddUObject(this, &US_FindGameScreen::OnJoinGameClicked);
        Btn_JoinGame->SetIsEnabled(false);
    }
    if (Btn_Back)
    {
        Btn_Back->OnClicked().AddUObject(this, &US_FindGameScreen::OnBackClicked);
    }
    if (Txt_Filter)
    {
        Txt_Filter->OnTextChanged.AddDynamic(this, &US_FindGameScreen::OnFilterTextChanged);
    }
    if (ServerListView)
    {
        // Corrected: Use OnItemSelectionChanged delegate if available, or OnObjectSelected if it's a UObject based list view.
        // CommonListView uses OnItemSelectionChanged typically.
        ServerListView->OnItemSelectionChanged().AddUObject(this, &US_FindGameScreen::OnServerSelected);
    }

    FindGameSessions();
}

void US_FindGameScreen::NativeDestruct()
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
            SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        }
    }
    Super::NativeDestruct();
}


void US_FindGameScreen::SetMenuManager_Implementation(UMenuManagerSubsystem* InMenuManager)
{
    MenuManager = InMenuManager;
}

void US_FindGameScreen::OnRefreshListClicked()
{
    FindGameSessions();
}

void US_FindGameScreen::OnFilterTextChanged(const FText& Text)
{
    if (SessionSearch.IsValid() && SessionSearch->SearchResults.Num() > 0)
    {
        PopulateServerList(SessionSearch->SearchResults);
    }
}


void US_FindGameScreen::OnJoinGameClicked()
{
    if (SelectedSessionResult.IsSet())
    {
        IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
        if (OnlineSub)
        {
            IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
            if (SessionInterface.IsValid())
            {
                JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &US_FindGameScreen::OnJoinSessionComplete);
                JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

                APlayerController* PC = GetOwningPlayer();
                if (PC && PC->GetLocalPlayer())
                {
                    if (!SessionInterface->JoinSession(*PC->GetLocalPlayer()->GetPreferredUniqueNetId(), NAME_GameSession, SelectedSessionResult.GetValue()))
                    {
                        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
                        UE_LOG(LogTemp, Error, TEXT("Failed to start JoinSession."));
                    }
                    else
                    {
                        UE_LOG(LogTemp, Log, TEXT("JoinSession call succeeded. Waiting for callback."));
                    }
                }
            }
        }
    }
}

void US_FindGameScreen::OnBackClicked()
{
    if (MenuManager)
    {
        MenuManager->CloseTopmostScreen();
    }
}

void US_FindGameScreen::OnServerSelected(UObject* Item)
{
    US_ServerRowData* RowData = Cast<US_ServerRowData>(Item);
    if (RowData && SessionSearch.IsValid())
    {
        // To find the correct FOnlineSessionSearchResult, we need to iterate or have a better mapping.
        // For simplicity, if RowData itself could store an index or if the ListView gives an index:
        int32 SelectedIdx = ServerListView->GetIndexForItem(Item); // Assumes ListView can provide this
        if (SessionSearch->SearchResults.IsValidIndex(SelectedIdx)) {
            SelectedSessionResult = SessionSearch->SearchResults[SelectedIdx];
            if (Btn_JoinGame) Btn_JoinGame->SetIsEnabled(true);
            UE_LOG(LogTemp, Log, TEXT("Server selected: %s"), *SelectedSessionResult.GetValue().Session.OwningUserName);
            return;
        }
    }
    SelectedSessionResult.Reset();
    if (Btn_JoinGame) Btn_JoinGame->SetIsEnabled(false);
}


void US_FindGameScreen::FindGameSessions()
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionSearch = MakeShareable(new FOnlineSessionSearch());
            SessionSearch->MaxSearchResults = 20;
            SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
            // SessionSearch->bIsLanQuery = false; 

            FindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &US_FindGameScreen::OnFindSessionsComplete);
            FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

            APlayerController* PC = GetOwningPlayer();
            if (PC && PC->GetLocalPlayer())
            {
                if (!SessionInterface->FindSessions(*PC->GetLocalPlayer()->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
                {
                    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
                    UE_LOG(LogTemp, Error, TEXT("Failed to start FindSessions."));
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("FindSessions call succeeded. Waiting for callback."));
                }
            }
        }
    }
}

void US_FindGameScreen::OnFindSessionsComplete(bool bWasSuccessful)
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
        }
    }

    if (bWasSuccessful && SessionSearch.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("FindSessions successful. Found %d sessions."), SessionSearch->SearchResults.Num());
        PopulateServerList(SessionSearch->SearchResults);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FindSessions failed or no sessions found."));
        if (ServerListView) ServerListView->ClearListItems();
    }
    SelectedSessionResult.Reset();
    if (Btn_JoinGame) Btn_JoinGame->SetIsEnabled(false);
}

void US_FindGameScreen::PopulateServerList(const TArray<FOnlineSessionSearchResult>& SearchResults)
{
    if (!ServerListView) return;
    ServerListView->ClearListItems();

    FString FilterString = Txt_Filter ? Txt_Filter->GetText().ToString().ToLower() : TEXT("");

    for (const FOnlineSessionSearchResult& Result : SearchResults)
    {
        FString ServerDisplayName = TEXT("Unnamed Server");
        Result.Session.SessionSettings.Get(FName(TEXT("SESSION_DISPLAY_NAME")), ServerDisplayName);

        if (!FilterString.IsEmpty() && !ServerDisplayName.ToLower().Contains(FilterString))
        {
            continue;
        }

        US_ServerRowData* Item = NewObject<US_ServerRowData>(this);
        if (Item)
        {
            Item->ServerName = ServerDisplayName;
            Result.Session.SessionSettings.Get(SETTING_MAPNAME, Item->MapName);
            Item->CurrentPlayers = Result.Session.SessionSettings.NumPublicConnections - Result.Session.NumOpenPublicConnections;
            Item->MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
            Item->Ping = Result.PingInMs;
            ServerListView->AddItem(Item);
        }
    }
}

void US_FindGameScreen::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        }
    }

    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        UE_LOG(LogTemp, Log, TEXT("JoinSession successful: %s"), *SessionName.ToString());
        APlayerController* PC = GetOwningPlayer();
        if (PC)
        {
            FString ConnectString;
            if (OnlineSub && OnlineSub->GetSessionInterface().IsValid() && OnlineSub->GetSessionInterface()->GetResolvedConnectString(SessionName, ConnectString))
            {
                PC->ClientTravel(ConnectString, TRAVEL_Absolute);
                if (MenuManager) MenuManager->ToggleMainMenu();
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to get connect string after joining session."));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to join session. Result: %d"), static_cast<int32>(Result));
    }
}