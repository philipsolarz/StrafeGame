// StrafeMultiplayer/Public/MultiplayerSessionTypes.h

#pragma once

#include "CoreMinimal.h"
#include "MultiplayerSessionTypes.generated.h"

// Keys for session settings
#define SESSION_KEY_GAME_MODE FName(TEXT("GAME_MODE"))
#define SESSION_KEY_MAP_NAME FName(TEXT("MAP_NAME"))
#define SESSION_KEY_GAME_SETTINGS FName(TEXT("GAME_SETTINGS"))
#define SESSION_KEY_IS_DEDICATED FName(TEXT("IS_DEDICATED")) // NEW KEY

UENUM(BlueprintType)
enum class EMultiplayerSessionResult : uint8
{
    Success,
    UnknownError,
    SubsystemNotInitialized,
    InvalidSession,
    Create_SessionAlreadyExists,
    Create_CreateRequestFailed,
    Find_FindRequestFailed,
    Find_NoResults,
    Join_CouldNotResolveConnectString,
    Join_JoinRequestFailed,
    Join_SessionIsFull,
    Join_SessionDoesNotExist,
    Destroy_DestroyRequestFailed
};

USTRUCT(BlueprintType)
struct FStrafeGameSessionSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Session Settings")
    int32 MaxPlayers;

    UPROPERTY(BlueprintReadWrite, Category = "Session Settings")
    FString GameMode;

    UPROPERTY(BlueprintReadWrite, Category = "Session Settings")
    FString MapName;

    UPROPERTY(BlueprintReadWrite, Category = "Session Settings")
    FString GameModeSettings;

    UPROPERTY(BlueprintReadWrite, Category = "Session Settings")
    bool bIsLANMatch;

    UPROPERTY(BlueprintReadWrite, Category = "Session Settings")
    bool bIsDedicated; // NEW FLAG

    FStrafeGameSessionSettings()
        : MaxPlayers(4),
        GameMode(TEXT("Arena")),
        MapName(TEXT("DefaultMap")),
        GameModeSettings(TEXT("")),
        bIsLANMatch(false),
        bIsDedicated(false) // Default to listen server
    {
    }
};