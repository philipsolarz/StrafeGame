// Source/StrafeGame/Private/GameModes/Strafe/S_StrafeGameMode.cpp
#include "GameModes/Strafe/S_StrafeGameMode.h"
#include "GameModes/Strafe/S_StrafeGameState.h"
#include "GameModes/Strafe/S_StrafePlayerState.h" // Specific PlayerState
#include "Player/S_PlayerController.h"
#include "Player/S_Character.h"
#include "GameModes/Strafe/S_StrafeManager.h" 
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AS_StrafeGameMode::AS_StrafeGameMode()
{
    // Set default classes specific to Strafe mode
    PlayerStateClass = AS_StrafePlayerState::StaticClass();
    GameStateClass = AS_StrafeGameState::StaticClass();
    // DefaultPawnClass and PlayerControllerClass inherited from AS_GameModeBase

    MatchDurationSeconds = 600; // 10 minutes default
    WarmupTime = 10.0f;
    PostMatchTime = 15.0f;

    // StrafeManagerClass should be set in Blueprint editor to your BP_S_StrafeManager
    CurrentStrafeManager = nullptr;
}

void AS_StrafeGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    SpawnStrafeManager();
}

void AS_StrafeGameMode::InitGameState()
{
    Super::InitGameState();
    AS_StrafeGameState* StrafeGS = GetStrafeGameState();
    if (StrafeGS)
    {
        StrafeGS->SetStrafeManager(CurrentStrafeManager); // Let GameState know about the manager
        StrafeGS->MatchDurationSeconds = MatchDurationSeconds; // Set replicated duration on GameState
    }
}

void AS_StrafeGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameMode: Player %s logged in."), *NewPlayer->GetName());

    AS_StrafePlayerState* StrafePS = NewPlayer->GetPlayerState<AS_StrafePlayerState>();
    if (StrafePS)
    {
        // Any specific initialization for StrafePlayerState on login
        if (CurrentStrafeManager)
        {
            // Ensure player is added to scoreboard/tracking in StrafeManager
            CurrentStrafeManager->UpdatePlayerInScoreboard(StrafePS);
        }
    }

    // If match is in warmup or already started, spawn pawn immediately
    if (IsMatchInProgress() || GetMatchState() == MatchState::WaitingToStart || GetMatchState() == FName(TEXT("Warmup")))
    {
        RestartPlayer(NewPlayer);
    }
}

bool AS_StrafeGameMode::PlayerCanRestart(APlayerController* Player)
{
    if (GetMatchState() == MatchState::InProgress || GetMatchState() == FName(TEXT("Warmup")))
    {
        return Super::PlayerCanRestart(Player);
    }
    if (GetMatchState() == MatchState::WaitingToStart && WarmupTime <= 0.f)
    {
        return Super::PlayerCanRestart(Player);
    }

    return false;
}

void AS_StrafeGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AS_StrafeGameMode::HandleMatchIsWaitingToStart()
{
    Super::HandleMatchIsWaitingToStart();
    UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameMode: Match is WaitingToStart."));

    if (CurrentStrafeManager)
    {
        CurrentStrafeManager->RefreshAndInitializeCheckpoints(); // Corrected
    }

    if (WarmupTime > 0.f)
    {
        StartWarmupTimer();
        AS_StrafeGameState* StrafeGS = GetStrafeGameState();
        if (StrafeGS)
        {
            StrafeGS->SetRemainingTime(FMath::CeilToInt(WarmupTime));
            StrafeGS->SetCurrentMatchStateName(FName(TEXT("Warmup")));
        }
    }
    else
    {
        StartMatch();
    }
}

void AS_StrafeGameMode::StartWarmupTimer()
{
    GetWorldTimerManager().SetTimer(WarmupTimerHandle, this, &AS_StrafeGameMode::OnWarmupTimerEnd, WarmupTime, false);
    UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameMode: Warmup timer started for %f seconds."), WarmupTime);
}

void AS_StrafeGameMode::OnWarmupTimerEnd()
{
    GetWorldTimerManager().ClearTimer(WarmupTimerHandle);
    UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameMode: Warmup ended. Starting match."));
    StartMatch();
}

void AS_StrafeGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();
    UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameMode: Match has started (InProgress)."));

    AS_StrafeGameState* StrafeGS = GetStrafeGameState();
    if (StrafeGS)
    {
        StrafeGS->SetRemainingTime(MatchDurationSeconds);
        StrafeGS->SetCurrentMatchStateName(NAME_None);
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->GetPawn() == nullptr)
        {
            RestartPlayer(PC);
        }
        AS_StrafePlayerState* StrafePS = PC ? PC->GetPlayerState<AS_StrafePlayerState>() : nullptr;
        if (StrafePS)
        {
            StrafePS->ServerStartRace();
        }
    }
    StartMainMatchTimer();
}

void AS_StrafeGameMode::StartMainMatchTimer()
{
    if (MatchDurationSeconds > 0)
    {
        GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &AS_StrafeGameMode::UpdateMainMatchTime, 1.0f, true);
        UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameMode: Main match timer started for %d seconds."), MatchDurationSeconds);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameMode: No match duration set, match will run indefinitely (or until other conditions)."));
    }
}

void AS_StrafeGameMode::UpdateMainMatchTime()
{
    AS_StrafeGameState* StrafeGS = GetStrafeGameState();
    if (StrafeGS && IsMatchInProgress())
    {
        int32 CurrentTime = StrafeGS->RemainingTime;
        CurrentTime--;
        StrafeGS->SetRemainingTime(CurrentTime);

        if (MatchDurationSeconds > 0 && CurrentTime <= 0)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameMode: Match time limit reached."));
            GetWorldTimerManager().ClearTimer(MatchTimerHandle);
            EndMatch();
        }
    }
    else if (IsMatchInProgress())
    {
        GetWorldTimerManager().ClearTimer(MatchTimerHandle);
    }
}

void AS_StrafeGameMode::HandleMatchHasEnded()
{
    Super::HandleMatchHasEnded();
    UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameMode: Match has ended (WaitingPostMatch)."));
    GetWorldTimerManager().ClearTimer(MatchTimerHandle);

    AS_StrafeGameState* StrafeGS = GetStrafeGameState();
    if (StrafeGS)
    {
        StrafeGS->SetCurrentMatchStateName(FName(TEXT("PostMatch")));
        if (CurrentStrafeManager)
        {
            // CurrentStrafeManager->FinalizeScoreboard(); // Hypothetical function
        }
    }

    if (PostMatchTime > 0.f)
    {
        StartPostMatchTimer();
        if (StrafeGS) StrafeGS->SetRemainingTime(FMath::CeilToInt(PostMatchTime));
    }
    else
    {
        OnPostMatchTimerEnd();
    }
}

void AS_StrafeGameMode::StartPostMatchTimer()
{
    GetWorldTimerManager().SetTimer(PostMatchTimerHandle, this, &AS_StrafeGameMode::OnPostMatchTimerEnd, PostMatchTime, false);
    UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameMode: Post-match timer started for %f seconds."), PostMatchTime);
}

void AS_StrafeGameMode::OnPostMatchTimerEnd()
{
    GetWorldTimerManager().ClearTimer(PostMatchTimerHandle);
    UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameMode: Post-match ended."));
    RestartGame();
}

void AS_StrafeGameMode::SpawnStrafeManager()
{
    if (HasAuthority() && StrafeManagerClass && !CurrentStrafeManager)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        CurrentStrafeManager = GetWorld()->SpawnActor<AS_StrafeManager>(StrafeManagerClass, SpawnParams);

        if (CurrentStrafeManager)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameMode: Spawned StrafeManager: %s"), *CurrentStrafeManager->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AS_StrafeGameMode: Failed to spawn StrafeManager from class %s."), *StrafeManagerClass->GetName());
        }
    }
    else if (CurrentStrafeManager)
    {
        UE_LOG(LogTemp, Log, TEXT("AS_StrafeGameMode: StrafeManager already exists or no class specified."));
    }
}

AS_StrafeGameState* AS_StrafeGameMode::GetStrafeGameState() const
{
    return GetGameState<AS_StrafeGameState>();
}