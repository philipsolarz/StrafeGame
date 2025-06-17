#pragma once

#include "CoreMinimal.h"
#include "GameModes/S_GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "S_StrafeGameMode.generated.h"

class AS_StrafeManager;
class AS_StrafePlayerState;
class AS_StrafeGameState;

UCLASS(Blueprintable, MinimalAPI)
class AS_StrafeGameMode : public AS_GameModeBase
{
    GENERATED_BODY()

public:
    AS_StrafeGameMode();

    //~ Begin AGameModeBase Interface
    // Called when the match is ready to start. The correct place to find all actors.
    virtual void StartPlay() override;

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void InitGameState() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void HandleMatchIsWaitingToStart() override;
    virtual void HandleMatchHasStarted() override;
    virtual void HandleMatchHasEnded() override;
    virtual bool PlayerCanRestart(APlayerController* Player) override;
    virtual void Tick(float DeltaSeconds) override;
    //~ End AGameModeBase Interface

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StrafeGameMode|Rules")
    int32 MatchDurationSeconds;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StrafeGameMode|Rules")
    float WarmupTime;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StrafeGameMode|Rules")
    float PostMatchTime;

    UPROPERTY(EditDefaultsOnly, Category = "StrafeGameMode|Actors")
    TSubclassOf<AS_StrafeManager> StrafeManagerClass;

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

    virtual void SpawnStrafeManager();

    AS_StrafeGameState* GetStrafeGameState() const;
};