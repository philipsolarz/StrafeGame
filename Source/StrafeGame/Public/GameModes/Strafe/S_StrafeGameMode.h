#pragma once

#include "CoreMinimal.h"
#include "GameModes/S_GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "S_StrafeGameMode.generated.h"

class AS_StrafeManager;       // Manages checkpoints and race logic
class AS_StrafePlayerState; // Specific PlayerState for this mode
class AS_StrafeGameState;   // Specific GameState for this mode

UCLASS(Blueprintable, MinimalAPI)
class AS_StrafeGameMode : public AS_GameModeBase
{
    GENERATED_BODY()

public:
    AS_StrafeGameMode();

    //~ Begin AGameModeBase Interface
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void InitGameState() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void HandleMatchIsWaitingToStart() override;
    virtual void HandleMatchHasStarted() override;
    virtual void HandleMatchHasEnded() override;
    virtual bool PlayerCanRestart(APlayerController* Player) override;
    virtual void Tick(float DeltaSeconds) override;
    //~ End AGameModeBase Interface

    //~ Begin AS_GameModeBase Interface
    /** Strafe mode doesn't typically have "kills" in the traditional sense affecting game score.
        Override if needed, but base might just handle respawn. */
        // virtual void OnPlayerKilled(AS_PlayerState* VictimPlayerState, AS_PlayerState* KillerPlayerState, AActor* KillingDamageCauser, AController* VictimController, AController* KillerController) override;
        //~ End AS_GameModeBase Interface

        /** Overall time limit for the Strafe match (main play phase). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StrafeGameMode|Rules")
    int32 MatchDurationSeconds;

    /** Duration of the warmup phase in seconds. 0 means no warmup or waits for ready. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StrafeGameMode|Rules")
    float WarmupTime;

    /** Duration of the post-match phase. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StrafeGameMode|Rules")
    float PostMatchTime;

    /** Class of the StrafeManager actor to spawn. */
    UPROPERTY(EditDefaultsOnly, Category = "StrafeGameMode|Actors")
    TSubclassOf<AS_StrafeManager> StrafeManagerClass;

    /** Instance of the StrafeManager for this match. */
    UPROPERTY(BlueprintReadOnly, Category = "StrafeGameMode")
    TObjectPtr<AS_StrafeManager> CurrentStrafeManager;

protected:
    FTimerHandle MatchTimerHandle;
    FTimerHandle WarmupTimerHandle;
    FTimerHandle PostMatchTimerHandle;

    void StartWarmupTimer();
    void OnWarmupTimerEnd();

    void StartMainMatchTimer();
    void UpdateMainMatchTime();

    void StartPostMatchTimer();
    void OnPostMatchTimerEnd();

    /** Spawns the StrafeManager actor. */
    virtual void SpawnStrafeManager();

    /** Helper to get the StrafeGameState. */
    AS_StrafeGameState* GetStrafeGameState() const;
};