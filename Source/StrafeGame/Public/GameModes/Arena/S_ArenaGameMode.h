// Source/StrafeGame/Public/GameModes/Arena/S_ArenaGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameModes/S_GameModeBase.h" 
#include "GameFramework/PlayerController.h" // Already included by S_GameModeBase likely
#include "S_ArenaGameMode.generated.h"

class AS_PlayerState;
class AS_ArenaPlayerState;
class AS_ArenaGameState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArenaMatchEndedDelegate, AS_PlayerState*, WinningPlayerState, FName, Reason);

UCLASS(Blueprintable, MinimalAPI)
class AS_ArenaGameMode : public AS_GameModeBase
{
    GENERATED_BODY()

public:
    AS_ArenaGameMode();

    //~ Begin AGameModeBase Interface
    virtual void InitGameState() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    virtual void HandleMatchIsWaitingToStart() override;
    virtual void HandleMatchHasStarted() override;
    virtual void HandleMatchHasEnded() override;
    virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
    virtual bool PlayerCanRestart(APlayerController* Player) override;
    virtual void Tick(float DeltaSeconds) override;
    //~ End AGameModeBase Interface

    //~ Begin AS_GameModeBase Interface
    virtual void OnPlayerKilled(AS_PlayerState* VictimPlayerState, AS_PlayerState* KillerPlayerState, AActor* KillingDamageCauser, AController* VictimController, AController* KillerController) override;
    //~ End AS_GameModeBase Interface

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ArenaGameMode|Rules")
    int32 MatchTimeLimitSeconds;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ArenaGameMode|Rules")
    int32 FragLimit;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ArenaGameMode|Rules")
    float WarmupTime;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ArenaGameMode|Rules")
    float PostMatchTime;

    UPROPERTY(BlueprintAssignable, Category = "ArenaGameMode|Events")
    FOnArenaMatchEndedDelegate OnArenaMatchEndedDelegate_Native;

protected:
    FTimerHandle MatchTimerHandle;
    FTimerHandle WarmupTimerHandle;
    FTimerHandle PostMatchTimerHandle;

    virtual void CheckMatchEndConditions();

    void StartMainMatchTimer();
    void UpdateMainMatchTime();

    void StartWarmupTimer();
    void OnWarmupTimerEnd();

    void StartPostMatchTimer();
    void OnPostMatchTimerEnd();

    AS_ArenaPlayerState* GetMatchWinner() const;
    AS_ArenaGameState* GetArenaGameState() const;
};