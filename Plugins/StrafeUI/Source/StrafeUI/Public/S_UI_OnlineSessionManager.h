// Plugins/StrafeUI/Source/StrafeUI/Public/S_UI_OnlineSessionManager.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "S_UI_OnlineSessionManager.generated.h"

/**
 * Delegate fired when the session state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionStateChanged, bool, bIsInSession);

/**
 * Centralized manager for online session operations
 * Handles session lifecycle and provides helper methods for common operations
 */
UCLASS()
class STRAFEUI_API US_UI_OnlineSessionManager : public UObject
{
    GENERATED_BODY()

public:
    /** Initialize the manager */
    void Initialize();

    /** Cleanup any active sessions and delegates */
    void Shutdown();

    /** Check if we're currently in a session */
    UFUNCTION(BlueprintCallable, Category = "Online Session")
    bool IsInSession() const;

    /** Get the current session name */
    UFUNCTION(BlueprintCallable, Category = "Online Session")
    FName GetCurrentSessionName() const { return CurrentSessionName; }

    /** Destroy the current session if one exists */
    UFUNCTION(BlueprintCallable, Category = "Online Session")
    void DestroyCurrentSession();

    /** Leave the current session (for clients) */
    UFUNCTION(BlueprintCallable, Category = "Online Session")
    void LeaveSession();

    /** Register a session with the online service (for dedicated servers) */
    UFUNCTION(BlueprintCallable, Category = "Online Session")
    void RegisterSession(const FString& ServerName, const FString& MapName, int32 MaxPlayers);

    /** Update session settings while in a session */
    UFUNCTION(BlueprintCallable, Category = "Online Session")
    void UpdateSessionSettings(const TMap<FName, FString>& NewSettings);

    /** Get a reference to the online session interface */
    IOnlineSessionPtr GetSessionInterface() const;

    /** Event fired when session state changes */
    UPROPERTY(BlueprintAssignable, Category = "Online Session")
    FOnSessionStateChanged OnSessionStateChanged;

private:
    /** Callbacks for session operations */
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
    void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnRegisterSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);

    /** Helper to clean up all session delegates */
    void ClearAllDelegates();

    /** The name of the current session */
    FName CurrentSessionName;

    /** Delegate handles for cleanup */
    FDelegateHandle DestroySessionCompleteDelegateHandle;
    FDelegateHandle EndSessionCompleteDelegateHandle;
    FDelegateHandle RegisterSessionCompleteDelegateHandle;
    FDelegateHandle UpdateSessionCompleteDelegateHandle;

    /** Cached session interface */
    IOnlineSessionPtr CachedSessionInterface;

    /** Flag to track if we're currently destroying a session */
    bool bIsDestroyingSession = false;
};