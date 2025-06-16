// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_OnlineSessionManager.cpp

#include "S_UI_OnlineSessionManager.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

// Define custom session settings keys
#define SETTING_MAPNAME FName(TEXT("MAPNAME"))
#define SETTING_GAMENAME FName(TEXT("GAMENAME"))

void US_UI_OnlineSessionManager::Initialize()
{
    // Cache the session interface
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        CachedSessionInterface = OnlineSubsystem->GetSessionInterface();
    }

    CurrentSessionName = NAME_GameSession;
}

void US_UI_OnlineSessionManager::Shutdown()
{
    ClearAllDelegates();

    // If we're in a session, try to destroy it
    if (IsInSession() && !bIsDestroyingSession)
    {
        DestroyCurrentSession();
    }
}

bool US_UI_OnlineSessionManager::IsInSession() const
{
    if (!CachedSessionInterface.IsValid())
    {
        return false;
    }

    return CachedSessionInterface->GetNamedSession(CurrentSessionName) != nullptr;
}

IOnlineSessionPtr US_UI_OnlineSessionManager::GetSessionInterface() const
{
    if (CachedSessionInterface.IsValid())
    {
        return CachedSessionInterface;
    }

    // Try to get it again if not cached
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        return OnlineSubsystem->GetSessionInterface();
    }

    return nullptr;
}

void US_UI_OnlineSessionManager::DestroyCurrentSession()
{
    if (!CachedSessionInterface.IsValid() || bIsDestroyingSession)
    {
        return;
    }

    if (!IsInSession())
    {
        UE_LOG(LogTemp, Warning, TEXT("DestroyCurrentSession called but no session exists"));
        return;
    }

    bIsDestroyingSession = true;

    // Bind the destroy complete delegate
    DestroySessionCompleteDelegateHandle = CachedSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
        FOnDestroySessionCompleteDelegate::CreateUObject(this, &US_UI_OnlineSessionManager::OnDestroySessionComplete)
    );

    // Destroy the session
    if (!CachedSessionInterface->DestroySession(CurrentSessionName))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to destroy session"));

        // Clean up since we won't get a callback
        CachedSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
        bIsDestroyingSession = false;
    }
}

void US_UI_OnlineSessionManager::LeaveSession()
{
    if (!CachedSessionInterface.IsValid())
    {
        return;
    }

    if (!IsInSession())
    {
        UE_LOG(LogTemp, Warning, TEXT("LeaveSession called but no session exists"));
        return;
    }

    // For clients, we end the session rather than destroy it
    EndSessionCompleteDelegateHandle = CachedSessionInterface->AddOnEndSessionCompleteDelegate_Handle(
        FOnEndSessionCompleteDelegate::CreateUObject(this, &US_UI_OnlineSessionManager::OnEndSessionComplete)
    );

    if (!CachedSessionInterface->EndSession(CurrentSessionName))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to end session"));

        // Clean up since we won't get a callback
        CachedSessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
    }
}

void US_UI_OnlineSessionManager::RegisterSession(const FString& ServerName, const FString& MapName, int32 MaxPlayers)
{
    if (!CachedSessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("No session interface available"));
        return;
    }

    // Create session settings for dedicated server
    TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());

    SessionSettings->NumPublicConnections = MaxPlayers;
    SessionSettings->NumPrivateConnections = 0;
    SessionSettings->bShouldAdvertise = true;
    SessionSettings->bAllowJoinInProgress = true;
    SessionSettings->bIsLANMatch = false;
    SessionSettings->bUsesPresence = false; // Dedicated servers don't use presence
    SessionSettings->bAllowInvites = true;
    SessionSettings->bAllowJoinViaPresence = false;
    SessionSettings->bIsDedicated = true;

    // Set custom properties
    SessionSettings->Set(SETTING_GAMENAME, ServerName, EOnlineDataAdvertisementType::ViaOnlineService);
    SessionSettings->Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineService);

    // Bind completion delegate
    RegisterSessionCompleteDelegateHandle = CachedSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &US_UI_OnlineSessionManager::OnRegisterSessionComplete)
    );

    // Create the session
    if (!CachedSessionInterface->CreateSession(0, CurrentSessionName, *SessionSettings))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to register dedicated server session"));

        // Clean up
        CachedSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(RegisterSessionCompleteDelegateHandle);
    }
}

void US_UI_OnlineSessionManager::UpdateSessionSettings(const TMap<FName, FString>& NewSettings)
{
    if (!CachedSessionInterface.IsValid() || !IsInSession())
    {
        return;
    }

    FOnlineSessionSettings* SessionSettings = CachedSessionInterface->GetSessionSettings(CurrentSessionName);
    if (!SessionSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get session settings for update"));
        return;
    }

    // Update the settings
    for (const auto& Setting : NewSettings)
    {
        SessionSettings->Set(Setting.Key, Setting.Value, EOnlineDataAdvertisementType::ViaOnlineService);
    }

    // Bind completion delegate
    UpdateSessionCompleteDelegateHandle = CachedSessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(
        FOnUpdateSessionCompleteDelegate::CreateUObject(this, &US_UI_OnlineSessionManager::OnUpdateSessionComplete)
    );

    // Update the session
    if (!CachedSessionInterface->UpdateSession(CurrentSessionName, *SessionSettings))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to update session"));

        // Clean up
        CachedSessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
    }
}

void US_UI_OnlineSessionManager::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (CachedSessionInterface.IsValid())
    {
        CachedSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    }

    bIsDestroyingSession = false;

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Session destroyed successfully"));
        OnSessionStateChanged.Broadcast(false);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to destroy session"));
    }
}

void US_UI_OnlineSessionManager::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (CachedSessionInterface.IsValid())
    {
        CachedSessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
    }

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Session ended successfully"));

        // After ending, destroy the session
        DestroyCurrentSession();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to end session"));
    }
}

void US_UI_OnlineSessionManager::OnRegisterSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (CachedSessionInterface.IsValid())
    {
        CachedSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(RegisterSessionCompleteDelegateHandle);
    }

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Dedicated server session registered successfully"));
        OnSessionStateChanged.Broadcast(true);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to register dedicated server session"));
    }
}

void US_UI_OnlineSessionManager::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (CachedSessionInterface.IsValid())
    {
        CachedSessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
    }

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Session updated successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to update session"));
    }
}

void US_UI_OnlineSessionManager::ClearAllDelegates()
{
    if (!CachedSessionInterface.IsValid())
    {
        return;
    }

    if (DestroySessionCompleteDelegateHandle.IsValid())
    {
        CachedSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    }

    if (EndSessionCompleteDelegateHandle.IsValid())
    {
        CachedSessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
    }

    if (RegisterSessionCompleteDelegateHandle.IsValid())
    {
        CachedSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(RegisterSessionCompleteDelegateHandle);
    }

    if (UpdateSessionCompleteDelegateHandle.IsValid())
    {
        CachedSessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
    }
}