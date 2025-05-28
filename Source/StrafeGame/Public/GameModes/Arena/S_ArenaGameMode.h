#pragma once

#include "CoreMinimal.h"
#include "GameModes/S_GameModeBase.h" // Inherit from our base GameMode
#include "GameFramework/PlayerController.h"
#include "S_ArenaGameMode.generated.h"

class AS_PlayerState;       // Forward declare base PlayerState
class AS_ArenaPlayerState;  // Forward declare Arena specific PlayerState
class AS_ArenaGameState;    // Forward declare Arena specific GameState

// Delegate for when the Arena match ends
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArenaMatchEndedDelegate, AS_PlayerState*, WinningPlayerState, FName, Reason);

UCLASS(Blueprintable, MinimalAPI)
class AS_ArenaGameMode : public AS_GameModeBase
{
    GENERATED_BODY()

public:
    AS_ArenaGameMode();

    //~ Begin AGameModeBase Interface
    virtual void InitGameState() override;
    /** Called after a player successfully logs in. */
    virtual void PostLogin(APlayerController* NewPlayer) override;
    /** Called when a player logs out or is disconnected. */
    virtual void Logout(AController* Exiting) override;
    /** Called when the match state is set to WaitingToStart */
    virtual void HandleMatchIsWaitingToStart() override;
    /** Called when the match state is set to InProgress */
    virtual void HandleMatchHasStarted() override;
    /** Called when the match state is set to WaitingPostMatch */
    virtual void HandleMatchHasEnded() override;
    /** Allow or disallow player login during different match states. */
    virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
    /** Can a player restart? Used for respawning. */
    virtual bool PlayerCanRestart(APlayerController* Player) override;
    /** Tick the GameMode. Used here for managing match timers. */
    virtual void Tick(float DeltaSeconds) override;
    //~ End AGameModeBase Interface

    //~ Begin AS_GameModeBase Interface
    /** Overridden to handle scoring for Arena mode. */
    virtual void OnPlayerKilled(AS_PlayerState* VictimPlayerState, AS_PlayerState* KillerPlayerState, AActor* KillingDamageCauser, AController* VictimController, AController* KillerController) override;
    //~ End AS_GameModeBase Interface

    /** Match time limit in seconds for the main play phase. 0 means no time limit. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ArenaGameMode|Rules")
    int32 MatchTimeLimitSeconds;

    /** Frag limit for the match. 0 means no frag limit. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ArenaGameMode|Rules")
    int32 FragLimit;

    /** Duration of the warmup phase in seconds. 0 means no warmup or waits for ready. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ArenaGameMode|Rules")
    float WarmupTime;

    /** Duration of the post-match phase before returning to lobby or new round. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ArenaGameMode|Rules")
    float PostMatchTime;

    /** Delegate broadcast when the Arena match ends. */
    UPROPERTY(BlueprintAssignable, Category = "ArenaGameMode|Events")
    FOnArenaMatchEndedDelegate OnArenaMatchEndedDelegate_Native; // Renamed to avoid clash with AGameMode's EndMatch

protected:
    /** Timer handle for the main match timer. */
    FTimerHandle MatchTimerHandle;
    /** Timer handle for the warmup phase. */
    FTimerHandle WarmupTimerHandle;
    /** Timer handle for the post-match phase. */
    FTimerHandle PostMatchTimerHandle;

    /** Checks if the match should end due to time limit or frag limit. */
    virtual void CheckMatchEndConditions();

    /** Starts the main match timer if MatchTimeLimitSeconds > 0. */
    void StartMainMatchTimer();
    /** Called by MatchTimerHandle to decrement time and check end conditions. */
    void UpdateMainMatchTime();

    /** Starts the warmup timer. */
    void StartWarmupTimer();
    /** Called when warmup timer ends. Transitions to InProgress. */
    void OnWarmupTimerEnd();

    /** Starts the post-match timer. */
    void StartPostMatchTimer();
    /** Called when post-match timer ends. Handles restarting the game or returning to lobby. */
    void OnPostMatchTimerEnd();

    /** Determines the winner of the match based on frags. Returns nullptr for a draw. */
    AS_ArenaPlayerState* GetMatchWinner() const;

    /** Helper to get the ArenaGameState. */
    AS_ArenaGameState* GetArenaGameState() const;
};