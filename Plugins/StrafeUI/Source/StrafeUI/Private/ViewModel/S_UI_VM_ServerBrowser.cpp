// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_VM_ServerBrowser.cpp

#include "ViewModel/S_UI_VM_ServerBrowser.h"
#include "S_UI_Subsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Data/S_UI_ScreenTypes.h"
#include "StrafeMultiplayer/Public/StrafeMultiplayerSubsystem.h"
#include "StrafeMultiplayer/Public/MultiplayerSessionTypes.h"

US_UI_VM_ServerBrowser::US_UI_VM_ServerBrowser()
{
	// Constructor
}

US_UI_VM_ServerBrowser::~US_UI_VM_ServerBrowser()
{
	// Clean up delegates if needed
	if (MultiplayerSubsystem)
	{
		// Delegates are automatically cleaned up when the object is destroyed
	}
}

void US_UI_VM_ServerBrowser::Initialize()
{
	// Get the StrafeMultiplayer subsystem and bind delegates
	if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		MultiplayerSubsystem = GameInstance->GetSubsystem<UStrafeMultiplayerSubsystem>();
		if (MultiplayerSubsystem)
		{
			// Bind to find sessions complete delegates
			MultiplayerSubsystem->OnFindLobbiesComplete.AddUObject(this, &US_UI_VM_ServerBrowser::OnFindSessionsComplete);
			MultiplayerSubsystem->OnFindDedicatedServersComplete.AddUObject(this, &US_UI_VM_ServerBrowser::OnFindSessionsComplete);

			// Bind to join session complete delegate
			MultiplayerSubsystem->OnJoinSessionComplete.AddUObject(this, &US_UI_VM_ServerBrowser::OnJoinSessionComplete);
		}
	}
}

void US_UI_VM_ServerBrowser::RequestServerListRefresh()
{
	if (!MultiplayerSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("No multiplayer subsystem found"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Refreshing server list..."));

	// Clear existing lists
	ServerList.Empty();
	AllFoundServers.Empty();
	BroadcastDataChanged();

	// Search for both lobbies and dedicated servers
	if (bSearchLAN)
	{
		// For LAN, just search for lobbies
		MultiplayerSubsystem->FindLobbies();
	}
	else
	{
		// For online, search for both
		MultiplayerSubsystem->FindLobbies();
		MultiplayerSubsystem->FindDedicatedServers();
	}
}

void US_UI_VM_ServerBrowser::OnFindSessionsComplete(const TArray<FOnlineSessionSearchResult>& SessionResults, EMultiplayerSessionResult Result)
{
	if (Result != EMultiplayerSessionResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session search failed. Result: %d"), (int32)Result);

		if (Result == EMultiplayerSessionResult::Find_NoResults)
		{
			// No servers found - show modal
			if (UWorld* World = GetWorld())
			{
				if (US_UI_Subsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
				{
					F_UIModalPayload Payload;
					Payload.Message = FText::FromString(TEXT("No game sessions found. Try creating your own!"));
					Payload.ModalType = E_UIModalType::OK;
					UISubsystem->RequestModal(Payload, FOnModalDismissedSignature());
				}
			}
		}
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Session search complete. Found %d sessions"), SessionResults.Num());

	// Process each found session
	for (const FOnlineSessionSearchResult& SearchResult : SessionResults)
	{
		US_UI_VM_ServerListEntry* NewEntry = NewObject<US_UI_VM_ServerListEntry>(this);

		// Store the full search result for joining later
		NewEntry->SessionSearchResult = SearchResult;

		// Extract basic info
		F_ServerInfo& ServerInfo = NewEntry->ServerInfo;

		// Get player counts
		ServerInfo.PlayerCount = SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;
		ServerInfo.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;

		// Get ping
		ServerInfo.Ping = SearchResult.PingInMs;

		// Get basic settings
		ServerInfo.bIsPrivate = !SearchResult.Session.SessionSettings.bShouldAdvertise;
		ServerInfo.bIsLAN = SearchResult.Session.SessionSettings.bIsLANMatch;

		// Get custom session data using StrafeMultiplayer keys
		FString GameMode;
		if (SearchResult.Session.SessionSettings.Get(FName(TEXT("GAME_MODE")), GameMode))
		{
			ServerInfo.GameMode = FText::FromString(GameMode);
		}
		else
		{
			ServerInfo.GameMode = FText::FromString(TEXT("Unknown"));
		}

		FString MapName;
		if (SearchResult.Session.SessionSettings.Get(FName(TEXT("MAP_NAME")), MapName))
		{
			ServerInfo.CurrentMap = MapName;
		}
		else
		{
			ServerInfo.CurrentMap = TEXT("Unknown");
		}

		// For server name, use the owner's name or a custom game name if available
		FString GameName;
		if (SearchResult.Session.SessionSettings.Get(FName(TEXT("GAME_NAME")), GameName))
		{
			ServerInfo.ServerName = FText::FromString(GameName);
		}
		else if (!SearchResult.Session.OwningUserName.IsEmpty())
		{
			ServerInfo.ServerName = FText::FromString(SearchResult.Session.OwningUserName + TEXT("'s Game"));
		}
		else
		{
			ServerInfo.ServerName = FText::FromString(TEXT("Unknown Server"));
		}

		// Check if it's a dedicated server
		bool bIsDedicated = false;
		SearchResult.Session.SessionSettings.Get(FName(TEXT("IS_DEDICATED")), bIsDedicated);

		if (bIsDedicated)
		{
			// Override the server name for dedicated servers
			ServerInfo.ServerName = FText::FromString(FString::Printf(TEXT("DEDI - %s (%s)"), *GameMode, *MapName));
		}

		AllFoundServers.Add(NewEntry);
	}

	// Apply filters to show results
	UpdateFilteredServerList();
}

void US_UI_VM_ServerBrowser::JoinSession(const FOnlineSessionSearchResult& SessionSearchResult)
{
	if (!MultiplayerSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("No multiplayer subsystem found"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Attempting to join session..."));
	MultiplayerSubsystem->JoinSession(SessionSearchResult);
}

void US_UI_VM_ServerBrowser::OnJoinSessionComplete(EMultiplayerSessionResult Result, const FString& ConnectString)
{
	if (Result == EMultiplayerSessionResult::Success)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully joined session. Connect string: %s"), *ConnectString);

		// Travel to the server
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to join session. Result: %d"), (int32)Result);

		// Show error modal
		FString ErrorMessage;
		switch (Result)
		{
		case EMultiplayerSessionResult::Join_SessionIsFull:
			ErrorMessage = TEXT("The session is full.");
			break;
		case EMultiplayerSessionResult::Join_SessionDoesNotExist:
			ErrorMessage = TEXT("The session no longer exists.");
			break;
		case EMultiplayerSessionResult::Join_CouldNotResolveConnectString:
			ErrorMessage = TEXT("Could not resolve connection address.");
			break;
		default:
			ErrorMessage = TEXT("Failed to join the session. Please try again.");
			break;
		}

		if (UWorld* World = GetWorld())
		{
			if (US_UI_Subsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
			{
				F_UIModalPayload Payload;
				Payload.Message = FText::FromString(ErrorMessage);
				Payload.ModalType = E_UIModalType::OK;
				UISubsystem->RequestModal(Payload, FOnModalDismissedSignature());
			}
		}
	}
}

void US_UI_VM_ServerBrowser::ApplyFilters()
{
	UpdateFilteredServerList();
}

void US_UI_VM_ServerBrowser::UpdateFilteredServerList()
{
	ServerList.Empty();

	// Apply filters to the full list
	for (const TObjectPtr<US_UI_VM_ServerListEntry>& Entry : AllFoundServers)
	{
		if (PassesFilters(Entry))
		{
			ServerList.Add(Entry->ServerInfo);
		}
	}

	// Notify UI of changes
	BroadcastDataChanged();
}

bool US_UI_VM_ServerBrowser::PassesFilters(const TObjectPtr<US_UI_VM_ServerListEntry>& Entry) const
{
	if (!Entry)
	{
		return false;
	}

	const F_ServerInfo& ServerInfo = Entry->ServerInfo;

	// Filter by server name
	if (!FilterServerName.IsEmpty())
	{
		if (!ServerInfo.ServerName.ToString().Contains(FilterServerName, ESearchCase::IgnoreCase))
		{
			return false;
		}
	}

	// Filter by game mode
	if (!FilterGameMode.IsEmpty())
	{
		if (!ServerInfo.GameMode.ToString().Contains(FilterGameMode, ESearchCase::IgnoreCase))
		{
			return false;
		}
	}

	// Filter by ping
	if (ServerInfo.Ping > FilterMaxPing)
	{
		return false;
	}

	// Filter full servers
	if (bFilterHideFullServers && ServerInfo.PlayerCount >= ServerInfo.MaxPlayers)
	{
		return false;
	}

	// Filter empty servers
	if (bFilterHideEmptyServers && ServerInfo.PlayerCount == 0)
	{
		return false;
	}

	// Filter private servers
	if (bFilterHidePrivateServers && ServerInfo.bIsPrivate)
	{
		return false;
	}

	return true;
}