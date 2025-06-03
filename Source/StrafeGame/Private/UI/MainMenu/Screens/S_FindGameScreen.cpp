// Source/StrafeGame/Private/UI/MainMenu/Screens/S_FindGameScreen.cpp
#include "UI/MainMenu/Screens/S_FindGameScreen.h"
#include "CommonButtonBase.h"
#include "Components/EditableTextBox.h"
#include "Components/ListView.h"
#include "UI/MainMenu/Widgets/S_ServerRowWidget.h"
#include "UI/MainMenu/MenuManagerSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h" // Required for GetPreferredUniqueNetId()

// Define for SETTING_MAPNAME if not available
#ifndef SETTING_MAPNAME
#define SETTING_MAPNAME FName(TEXT("MAPNAME"))
#endif

// Define for SEARCH_PRESENCE if not available (common key for presence-based searches)
#ifndef SEARCH_PRESENCE
#define SEARCH_PRESENCE FName(TEXT("PRESENCESEARCH"))
#endif


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
        // Ensure the ListView is configured in BP to use US_ServerRowData for list item data object
        // and WBP_ServerRow (derived from US_ServerRowWidget) for entry widget class.
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
                if (PC && PC->GetLocalPlayer()) // Check if LocalPlayer is valid
                {
                    const FUniqueNetIdRepl LocalPlayerNetId = PC->GetLocalPlayer()->GetPreferredUniqueNetId();
                    if (!LocalPlayerNetId.IsValid())
                    {
                        UE_LOG(LogTemp, Error, TEXT("JoinSession: LocalPlayerNetId is not valid."));
                        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
                        return;
                    }

                    if (!SessionInterface->JoinSession(*LocalPlayerNetId, NAME_GameSession, SelectedSessionResult.GetValue()))
                    {
                        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
                        UE_LOG(LogTemp, Error, TEXT("Failed to start JoinSession."));
                    }
                    else
                    {
                        UE_LOG(LogTemp, Log, TEXT("JoinSession call succeeded. Waiting for callback."));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("JoinSession: OwningPlayerController or LocalPlayer is null."));
                    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
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
        int32 FoundIdx = -1;
        for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
        {
            const auto& Result = SessionSearch->SearchResults[i];
            FString ServerDisplayName = TEXT("Unnamed Server");
            Result.Session.SessionSettings.Get(FName(TEXT("SESSION_DISPLAY_NAME")), ServerDisplayName);

            FString MapNameFromSession;
            Result.Session.SessionSettings.Get(SETTING_MAPNAME, MapNameFromSession);

            // A more reliable way to link RowData to SearchResult would be to store SessionId in RowData
            // For now, we'll try to match on name and map as a simpler approach.
            if (ServerDisplayName == RowData->ServerName && MapNameFromSession == RowData->MapName)
            {
                FoundIdx = i;
                break;
            }
        }

        if (SessionSearch->SearchResults.IsValidIndex(FoundIdx)) {
            SelectedSessionResult = SessionSearch->SearchResults[FoundIdx];
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
    if (Btn_RefreshList) Btn_RefreshList->SetIsEnabled(false);
    if (ServerListView) ServerListView->ClearListItems();
    SelectedSessionResult.Reset();
    if (Btn_JoinGame) Btn_JoinGame->SetIsEnabled(false);


    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionSearch = MakeShareable(new FOnlineSessionSearch());
            SessionSearch->MaxSearchResults = 20;
            SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
            SessionSearch->bIsLanQuery = false;

            FindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &US_FindGameScreen::OnFindSessionsComplete);
            FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

            APlayerController* PC = GetOwningPlayer();
            if (PC && PC->GetLocalPlayer()) // Check if LocalPlayer is valid
            {
                const FUniqueNetIdRepl LocalPlayerNetId = PC->GetLocalPlayer()->GetPreferredUniqueNetId();
                if (!LocalPlayerNetId.IsValid())
                {
                    UE_LOG(LogTemp, Error, TEXT("FindSessions: LocalPlayerNetId is not valid."));
                    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
                    if (Btn_RefreshList) Btn_RefreshList->SetIsEnabled(true);
                    return;
                }

                if (!SessionInterface->FindSessions(*LocalPlayerNetId, SessionSearch.ToSharedRef()))
                {
                    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
                    UE_LOG(LogTemp, Error, TEXT("Failed to start FindSessions."));
                    if (Btn_RefreshList) Btn_RefreshList->SetIsEnabled(true);
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("FindSessions call succeeded. Waiting for callback."));
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("FindSessions: OwningPlayerController or LocalPlayer is null."));
                SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
                if (Btn_RefreshList) Btn_RefreshList->SetIsEnabled(true);
            }
        }
    }
    else
    {
        if (Btn_RefreshList) Btn_RefreshList->SetIsEnabled(true);
    }
}

void US_FindGameScreen::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (Btn_RefreshList) Btn_RefreshList->SetIsEnabled(true);

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