#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h" // Using AGameMode for its existing match state machine
#include "S_GameModeBase.generated.h"

class AS_PlayerState; // Forward declaration

/**
 * Base GameMode for StrafeGame.
 * Sets common default classes and provides hooks for game-mode specific logic.
 */
UCLASS(Abstract, MinimalAPI) // MinimalAPI to avoid exporting too much if not needed
class AS_GameModeBase : public AGameMode
{
    GENERATED_BODY()

public:
    AS_GameModeBase();

    //~ Begin AGameModeBase Interface
    /** Called after a player successfully logs in. */
    virtual void PostLogin(APlayerController* NewPlayer) override;

    /** Called when a player logs out or is disconnected. */
    virtual void Logout(AController* Exiting) override;

    /** Called to initialize the GameState actor. */
    virtual void InitGameState() override;

    /** Handles initializing the game session, occurs before any GameMode functions. */
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    //~ End AGameModeBase Interface


    /**
     * Called when a player's controlled pawn is killed.
     * This is a central place to handle scoring, respawn logic, etc.
     * @param VictimPlayerState PlayerState of the player who was killed.
     * @param KillerPlayerState PlayerState of the player who did the killing (can be null or VictimPlayerState for suicides/environment).
     * @param KillingDamageCauser The actor that directly caused the damage (e.g., projectile, weapon).
     * @param VictimController Controller of the pawn that was killed.
     * @param KillerController Controller of the pawn that did the killing.
     */
    UFUNCTION(BlueprintCallable, Category = "GameMode")
    virtual void OnPlayerKilled(
        AS_PlayerState* VictimPlayerState,
        AS_PlayerState* KillerPlayerState,
        AActor* KillingDamageCauser,
        AController* VictimController,
        AController* KillerController
    );

protected:
    /**
     * Called during PostLogin to perform any game-specific player initialization.
     * @param NewPlayer The controller for the new player.
     */
    virtual void InitializePlayer(APlayerController* NewPlayer);

    /** Handles player respawning. Finds a player start and calls RestartPlayer. */
    virtual void RequestRespawn(AController* PlayerToRespawn);

    /** Timer handle for delayed respawn. */
    FTimerHandle RespawnTimerHandle;

    /** Struct to keep track of players needing respawn if there's a delay. */
    struct FPendingRespawn
    {
        AController* Controller;
        FTimerDelegate RespawnDelegate;
    };
    TMap<AController*, FPendingRespawn> PendingRespawns;

    /** Delay before respawning a player after death. */
    UPROPERTY(EditDefaultsOnly, Category = "GameMode|Respawn")
    float RespawnDelay;

    /** Actual function to respawn player, called after delay. */
    virtual void ProcessRespawn(AController* PlayerToRespawn);

    // --- Match State Handling ---
    // Unreal's AGameMode already has a robust match state machine (MatchState property and functions like HandleMatchIsWaitingToStart, HandleMatchHasStarted, HandleMatchHasEnded).
    // We will override these in derived game modes (AS_ArenaGameMode, AS_StrafeGameMode) to implement specific logic for warmup, main play, and post-match states.

    // Example: Common function that might be called when game transitions to InProgress
    // virtual void OnMatchStarted();

    // Example: Common function that might be called when game transitions to WaitingPostMatch
    // virtual void OnMatchEnded(AS_PlayerState* WinnerPlayerState = nullptr);
};