// StrafeMultiplayer/Public/StrafeMultiplayerSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionTypes.h"
#include "StrafeMultiplayerSubsystem.generated.h"

// Custom delegates using the new result enums
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStrafeOnCreateSessionComplete, EMultiplayerSessionResult, Result);
DECLARE_MULTICAST_DELEGATE_TwoParams(FStrafeOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& /*SessionResults*/, EMultiplayerSessionResult /*Result*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FStrafeOnJoinSessionComplete, EMultiplayerSessionResult /*Result*/, const FString& /*ConnectString*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStrafeOnDestroySessionComplete, EMultiplayerSessionResult, Result);


UCLASS()
class STRAFEMULTIPLAYER_API UStrafeMultiplayerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStrafeMultiplayerSubsystem();

    void CreateSession(const FStrafeGameSessionSettings& SessionSettings);
    void FindLobbies();
    void FindDedicatedServers();
    void JoinSession(const FOnlineSessionSearchResult& SessionResult);
    void DestroySession();

    // Delegate Handles now use the corrected, unique names
    FStrafeOnCreateSessionComplete OnCreateSessionComplete;
    FStrafeOnFindSessionsComplete OnFindLobbiesComplete;
    FStrafeOnFindSessionsComplete OnFindDedicatedServersComplete;
    FStrafeOnJoinSessionComplete OnJoinSessionComplete;
    FStrafeOnDestroySessionComplete OnDestroySessionComplete;

protected:
    void OnCreateSessionComplete_Internal(FName SessionName, bool bWasSuccessful);
    void OnFindLobbiesComplete_Internal(bool bWasSuccessful);
    void OnFindDedicatedServersComplete_Internal(bool bWasSuccessful);
    void OnJoinSessionComplete_Internal(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void OnDestroySessionComplete_Internal(FName SessionName, bool bWasSuccessful);

private:
    bool IsSessionInterfaceValid();

    IOnlineSessionPtr SessionInterface;
    TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
    TSharedPtr<FOnlineSessionSearch> LastLobbySearch;
    TSharedPtr<FOnlineSessionSearch> LastDedicatedSearch;

    FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    FOnFindSessionsCompleteDelegate FindLobbiesCompleteDelegate;
    FDelegateHandle FindLobbiesCompleteDelegateHandle;
    FOnFindSessionsCompleteDelegate FindDedicatedServersCompleteDelegate;
    FDelegateHandle FindDedicatedServersCompleteDelegateHandle;
    FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
    FDelegateHandle JoinSessionCompleteDelegateHandle;
    FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
    FDelegateHandle DestroySessionCompleteDelegateHandle;

    bool bCreateSessionOnDestroy{ false };
    FStrafeGameSessionSettings CreateSessionOnDestroy_Settings;
};