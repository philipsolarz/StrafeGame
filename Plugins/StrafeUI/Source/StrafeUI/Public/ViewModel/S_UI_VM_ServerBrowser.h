// Plugins/StrafeUI/Source/StrafeUI/Public/ViewModel/S_UI_VM_ServerBrowser.h

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/S_UI_ViewModelBase.h"
#include "OnlineSessionSettings.h"
#include "StrafeMultiplayer/Public/MultiplayerSessionTypes.h"
#include "S_UI_VM_ServerBrowser.generated.h"

class UStrafeMultiplayerSubsystem;

/**
 * @struct F_ServerInfo
 * @brief Contains information about a single game server.
 */
USTRUCT(BlueprintType)
struct F_ServerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	FText ServerName;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	FText GameMode;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	FString CurrentMap;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	FText Description;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	int32 PlayerCount;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	int32 MaxPlayers;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	int32 Ping;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	bool bIsPrivate;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	bool bIsLAN;
};

/**
 * @class US_UI_VM_ServerListEntry
 * @brief A UObject wrapper for F_ServerInfo to be used with UListView.
 * The widget entry for the list view should implement IUserObjectListEntry
 * and cast the UObject to this class.
 */
UCLASS()
class STRAFEUI_API US_UI_VM_ServerListEntry : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	F_ServerInfo ServerInfo;

	/** The full session search result needed for joining */
	FOnlineSessionSearchResult SessionSearchResult;
};

/**
 * @class US_UI_VM_ServerBrowser
 * @brief ViewModel for the Server Browser screen.
 *
 * Manages the list of servers and handles refresh requests.
 */
UCLASS(BlueprintType)
class STRAFEUI_API US_UI_VM_ServerBrowser : public US_UI_ViewModelBase
{
	GENERATED_BODY()

public:
	US_UI_VM_ServerBrowser();
	virtual ~US_UI_VM_ServerBrowser();

	/**
	 * Initialize the view model
	 */
	void Initialize();

	/**
	 * The list of servers to be displayed in the UI.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Server Browser")
	TArray<F_ServerInfo> ServerList;

	/**
	 * Sends a request to refresh the server list.
	 * This would typically trigger an async call to a backend or online subsystem.
	 */
	UFUNCTION(BlueprintCallable, Category = "Server Browser")
	void RequestServerListRefresh();

	/**
	 * Joins the selected server session.
	 * @param SessionSearchResult The search result containing session info
	 */
	void JoinSession(const FOnlineSessionSearchResult& SessionSearchResult);

	// Filter properties
	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	FString FilterServerName;

	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	FString FilterGameMode;

	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	bool bFilterHideFullServers = false;

	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	bool bFilterHideEmptyServers = false;

	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	bool bFilterHidePrivateServers = false;

	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	int32 FilterMaxPing = 999;

	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	bool bSearchLAN = false;

	/** Apply current filters and refresh the displayed list */
	UFUNCTION(BlueprintCallable, Category = "Server Browser")
	void ApplyFilters();

	// Friend class to allow FindGameWidget to access AllFoundServers
	friend class US_UI_FindGameWidget;

private:
	/** Callback for when session search completes */
	void OnFindSessionsComplete(const TArray<FOnlineSessionSearchResult>& SessionResults, EMultiplayerSessionResult Result);

	/** Callback for when join session completes */
	void OnJoinSessionComplete(EMultiplayerSessionResult Result, const FString& ConnectString);

	/** Cached list of all found servers before filtering */
	UPROPERTY()
	TArray<TObjectPtr<US_UI_VM_ServerListEntry>> AllFoundServers;

	/** Reference to the multiplayer subsystem */
	UPROPERTY()
	TObjectPtr<UStrafeMultiplayerSubsystem> MultiplayerSubsystem;

	/** Updates the visible server list based on current filters */
	void UpdateFilteredServerList();

	/** Checks if a server passes the current filter criteria */
	bool PassesFilters(const TObjectPtr<US_UI_VM_ServerListEntry>& Entry) const;
};