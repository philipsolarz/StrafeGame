#include "GameModes/S_GameModeBase.h"
#include "Player/S_Character.h"
#include "Player/S_PlayerController.h"
#include "Player/S_PlayerState.h"
#include "GameModes/S_GameStateBase.h" // Will be our base GameState
#include "TimerManager.h"           // For FTimerManager
#include "Engine/World.h"             // For GetWorld()
#include "Kismet/GameplayStatics.h"   // For UGameplayStatics

AS_GameModeBase::AS_GameModeBase()
{
    // Set default classes common to all game modes
    PlayerControllerClass = AS_PlayerController::StaticClass();
    DefaultPawnClass = AS_Character::StaticClass();
    PlayerStateClass = AS_PlayerState::StaticClass(); // Base PlayerState
    GameStateClass = AS_GameStateBase::StaticClass(); // Base GameState
    // HUDClass will be set by specific game modes if needed, or a default one here

    RespawnDelay = 3.0f; // Default respawn delay
}

void AS_GameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    // Game-wide initialization that occurs before any other GameMode functions
    // UE_LOG(LogTemp, Log, TEXT("AS_GameModeBase::InitGame - Map: %s, Options: %s"), *MapName, *Options);
}

bool AS_GameModeBase::PlayerCanRestart(APlayerController* Player)
{
    return false;
}

void AS_GameModeBase::InitGameState()
{
    Super::InitGameState();
    // Initialize properties on the GameState.
    // AS_GameStateBase* GS = GetGameState<AS_GameStateBase>();
    // if (GS)
    // {
    //     // Set any common initial state here
    // }
}

void AS_GameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    InitializePlayer(NewPlayer);
    UE_LOG(LogTemp, Log, TEXT("AS_GameModeBase::PostLogin - Player %s logged in."), *NewPlayer->GetName());
}

void AS_GameModeBase::Logout(AController* Exiting)
{
    UE_LOG(LogTemp, Log, TEXT("AS_GameModeBase::Logout - Player %s logged out."), *Exiting->GetName());
    PendingRespawns.Remove(Exiting); // Remove from pending respawn list if they logout
    Super::Logout(Exiting);
}

void AS_GameModeBase::InitializePlayer(APlayerController* NewPlayer)
{
    if (NewPlayer)
    {
        AS_PlayerState* PS = NewPlayer->GetPlayerState<AS_PlayerState>();
        if (PS)
        {
            // Any common player state initialization that isn't handled by AS_PlayerState::BeginPlay
            // or specific game mode PostLogin.
            // For example, if you need to assign a default team or faction common to all modes.
        }
    }
}

void AS_GameModeBase::OnPlayerKilled(AS_PlayerState* VictimPlayerState, AS_PlayerState* KillerPlayerState, AActor* KillingDamageCauser, AController* VictimController, AController* KillerController)
{
    // Base implementation, can be overridden by specific game modes for scoring etc.
    UE_LOG(LogTemp, Log, TEXT("AS_GameModeBase::OnPlayerKilled: Victim %s, Killer: %s"),
        VictimPlayerState ? *VictimPlayerState->GetPlayerName() : TEXT("None"),
        KillerPlayerState ? *KillerPlayerState->GetPlayerName() : TEXT("None")
    );

    if (VictimController)
    {
        RequestRespawn(VictimController);
    }
}

void AS_GameModeBase::RequestRespawn(AController* PlayerToRespawn)
{
    if (!PlayerToRespawn || PendingRespawns.Contains(PlayerToRespawn))
    {
        return; // Already pending respawn or invalid controller
    }

    // If instant respawn, or specific conditions met
    if (RespawnDelay <= 0.0f)
    {
        ProcessRespawn(PlayerToRespawn);
    }
    else // Delayed respawn
    {
        FPendingRespawn& Pending = PendingRespawns.Add(PlayerToRespawn);
        Pending.Controller = PlayerToRespawn;
        Pending.RespawnDelegate.BindUObject(this, &AS_GameModeBase::ProcessRespawn, PlayerToRespawn);
        GetWorldTimerManager().SetTimer(RespawnTimerHandle, Pending.RespawnDelegate, RespawnDelay, false);
        UE_LOG(LogTemp, Log, TEXT("AS_GameModeBase::RequestRespawn: %s will respawn in %f seconds."), *PlayerToRespawn->GetName(), RespawnDelay);
    }
}

void AS_GameModeBase::ProcessRespawn(AController* PlayerToRespawn)
{
    if (PlayerToRespawn)
    {
        UE_LOG(LogTemp, Log, TEXT("AS_GameModeBase::ProcessRespawn: Respawning %s."), *PlayerToRespawn->GetName());
        PendingRespawns.Remove(PlayerToRespawn);

        // Find a player start
        AActor* PlayerStart = FindPlayerStart(PlayerToRespawn);
        RestartPlayerAtPlayerStart(PlayerToRespawn, PlayerStart);

        // Potentially trigger an event for the respawned player
        // AS_PlayerController* PC = Cast<AS_PlayerController>(PlayerToRespawn);
        // if (PC) PC->ClientOnRespawned(); // Example client RPC on PlayerController
    }
}