// StrafeMultiplayer/Private/StrafeMultiplayerSubsystem.cpp

#include "StrafeMultiplayerSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h" // Included for robust access to GetWorld() if needed, though subsystem's GetWorld() is fine.

DEFINE_LOG_CATEGORY_STATIC(LogStrafeMultiplayer, Log, All);

UStrafeMultiplayerSubsystem::UStrafeMultiplayerSubsystem() :
    CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete_Internal)),
    FindLobbiesCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindLobbiesComplete_Internal)),
    FindDedicatedServersCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindDedicatedServersComplete_Internal)),
    JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete_Internal)),
    DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete_Internal))
{
}

bool UStrafeMultiplayerSubsystem::IsSessionInterfaceValid()
{
    if (!SessionInterface)
    {
        IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
        if (Subsystem)
        {
            SessionInterface = Subsystem->GetSessionInterface();
        }
    }
    return SessionInterface.IsValid();
}

void UStrafeMultiplayerSubsystem::CreateSession(const FStrafeGameSessionSettings& SessionSettings)
{
    if (!IsSessionInterfaceValid())
    {
        OnCreateSessionComplete.Broadcast(EMultiplayerSessionResult::SubsystemNotInitialized);
        return;
    }

    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession != nullptr)
    {
        bCreateSessionOnDestroy = true;
        CreateSessionOnDestroy_Settings = SessionSettings;
        DestroySession();
        return;
    }

    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

    LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
    LastSessionSettings->bIsLANMatch = SessionSettings.bIsLANMatch;
    LastSessionSettings->NumPublicConnections = SessionSettings.MaxPlayers;
    LastSessionSettings->bAllowJoinInProgress = true;
    LastSessionSettings->bAllowJoinViaPresence = true;
    LastSessionSettings->bShouldAdvertise = true;
    LastSessionSettings->bUsesPresence = true;
    LastSessionSettings->bUseLobbiesIfAvailable = true;
    LastSessionSettings->BuildUniqueId = 1;

    LastSessionSettings->Set(SESSION_KEY_GAME_MODE, SessionSettings.GameMode, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    LastSessionSettings->Set(SESSION_KEY_MAP_NAME, SessionSettings.MapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    LastSessionSettings->Set(SESSION_KEY_GAME_SETTINGS, SessionSettings.GameModeSettings, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    LastSessionSettings->Set(SESSION_KEY_IS_DEDICATED, SessionSettings.bIsDedicated, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    // #################### API FIX ####################
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        OnCreateSessionComplete.Broadcast(EMultiplayerSessionResult::UnknownError);
        return;
    }

    if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        OnCreateSessionComplete.Broadcast(EMultiplayerSessionResult::Create_CreateRequestFailed);
    }
}

void UStrafeMultiplayerSubsystem::FindLobbies()
{
    if (!IsSessionInterfaceValid())
    {
        OnFindLobbiesComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), EMultiplayerSessionResult::SubsystemNotInitialized);
        return;
    }

    // #################### API FIX ####################
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        OnFindLobbiesComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), EMultiplayerSessionResult::UnknownError);
        return;
    }

    FindLobbiesCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindLobbiesCompleteDelegate);

    LastLobbySearch = MakeShareable(new FOnlineSessionSearch());
    LastLobbySearch->MaxSearchResults = 10000;
    LastLobbySearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
    LastLobbySearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

    if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastLobbySearch.ToSharedRef()))
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindLobbiesCompleteDelegateHandle);
        OnFindLobbiesComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), EMultiplayerSessionResult::Find_FindRequestFailed);
    }
}

void UStrafeMultiplayerSubsystem::FindDedicatedServers()
{
    if (!IsSessionInterfaceValid())
    {
        OnFindDedicatedServersComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), EMultiplayerSessionResult::SubsystemNotInitialized);
        return;
    }

    // #################### API FIX ####################
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        OnFindDedicatedServersComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), EMultiplayerSessionResult::UnknownError);
        return;
    }

    FindDedicatedServersCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindDedicatedServersCompleteDelegate);

    LastDedicatedSearch = MakeShareable(new FOnlineSessionSearch());
    LastDedicatedSearch->MaxSearchResults = 1000;
    LastDedicatedSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";

    if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastDedicatedSearch.ToSharedRef()))
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindDedicatedServersCompleteDelegateHandle);
        OnFindDedicatedServersComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), EMultiplayerSessionResult::Find_FindRequestFailed);
    }
}


void UStrafeMultiplayerSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
    if (!IsSessionInterfaceValid())
    {
        OnJoinSessionComplete.Broadcast(EMultiplayerSessionResult::SubsystemNotInitialized, "");
        return;
    }

    // #################### API FIX ####################
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        OnJoinSessionComplete.Broadcast(EMultiplayerSessionResult::UnknownError, "");
        return;
    }

    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

    if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        OnJoinSessionComplete.Broadcast(EMultiplayerSessionResult::Join_JoinRequestFailed, "");
    }
}

void UStrafeMultiplayerSubsystem::DestroySession()
{
    if (!IsSessionInterfaceValid())
    {
        OnDestroySessionComplete.Broadcast(EMultiplayerSessionResult::SubsystemNotInitialized);
        return;
    }

    DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

    if (!SessionInterface->DestroySession(NAME_GameSession))
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
        OnDestroySessionComplete.Broadcast(EMultiplayerSessionResult::Destroy_DestroyRequestFailed);
    }
}

void UStrafeMultiplayerSubsystem::OnCreateSessionComplete_Internal(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    }
    OnCreateSessionComplete.Broadcast(bWasSuccessful ? EMultiplayerSessionResult::Success : EMultiplayerSessionResult::Create_CreateRequestFailed);
}

void UStrafeMultiplayerSubsystem::OnFindLobbiesComplete_Internal(bool bWasSuccessful)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindLobbiesCompleteDelegateHandle);
    }

    if (!bWasSuccessful || !LastLobbySearch.IsValid())
    {
        OnFindLobbiesComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), EMultiplayerSessionResult::Find_FindRequestFailed);
        return;
    }

    if (LastLobbySearch->SearchResults.Num() == 0)
    {
        OnFindLobbiesComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), EMultiplayerSessionResult::Find_NoResults);
        return;
    }

    OnFindLobbiesComplete.Broadcast(LastLobbySearch->SearchResults, EMultiplayerSessionResult::Success);
}

void UStrafeMultiplayerSubsystem::OnFindDedicatedServersComplete_Internal(bool bWasSuccessful)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindDedicatedServersCompleteDelegateHandle);
    }

    if (!bWasSuccessful || !LastDedicatedSearch.IsValid())
    {
        OnFindDedicatedServersComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), EMultiplayerSessionResult::Find_FindRequestFailed);
        return;
    }

    if (LastDedicatedSearch->SearchResults.Num() == 0)
    {
        OnFindDedicatedServersComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), EMultiplayerSessionResult::Find_NoResults);
        return;
    }

    OnFindDedicatedServersComplete.Broadcast(LastDedicatedSearch->SearchResults, EMultiplayerSessionResult::Success);
}

void UStrafeMultiplayerSubsystem::OnJoinSessionComplete_Internal(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
    }

    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        FString Address;
        if (SessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
        {
            OnJoinSessionComplete.Broadcast(EMultiplayerSessionResult::Success, Address);
        }
        else
        {
            OnJoinSessionComplete.Broadcast(EMultiplayerSessionResult::Join_CouldNotResolveConnectString, "");
        }
    }
    else
    {
        EMultiplayerSessionResult ErrorResult = EMultiplayerSessionResult::Join_JoinRequestFailed;
        if (Result == EOnJoinSessionCompleteResult::SessionIsFull) ErrorResult = EMultiplayerSessionResult::Join_SessionIsFull;
        else if (Result == EOnJoinSessionCompleteResult::SessionDoesNotExist) ErrorResult = EMultiplayerSessionResult::Join_SessionDoesNotExist;

        OnJoinSessionComplete.Broadcast(ErrorResult, "");
    }
}

void UStrafeMultiplayerSubsystem::OnDestroySessionComplete_Internal(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    }

    if (bWasSuccessful && bCreateSessionOnDestroy)
    {
        bCreateSessionOnDestroy = false;
        CreateSession(CreateSessionOnDestroy_Settings);
    }

    OnDestroySessionComplete.Broadcast(bWasSuccessful ? EMultiplayerSessionResult::Success : EMultiplayerSessionResult::Destroy_DestroyRequestFailed);
}