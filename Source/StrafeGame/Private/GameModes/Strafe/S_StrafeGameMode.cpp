// Source/StrafeGame/Private/GameModes/Strafe/S_StrafeGameMode.cpp
#include "GameModes/Strafe/S_StrafeGameMode.h"
#include "GameModes/Strafe/S_StrafeGameState.h"
#include "GameModes/Strafe/S_StrafePlayerState.h"
#include "Player/S_PlayerController.h"
#include "Player/S_Character.h"
#include "GameModes/Strafe/S_StrafeManager.h" 
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AS_StrafeGameMode::AS_StrafeGameMode()
{
    PlayerStateClass = AS_StrafePlayerState::StaticClass();
    GameStateClass = AS_StrafeGameState::StaticClass();
    MatchDurationSeconds = 600;
    WarmupTime = 10.0f;
    PostMatchTime = 15.0f;
    CurrentStrafeManager = nullptr;

    // Set default StrafeManager class to ensure it's always available
    StrafeManagerClass = AS_StrafeManager::StaticClass();
}

void AS_StrafeGameMode::StartPlay()
{
    Super::StartPlay();

    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode::StartPlay: Beginning StartPlay. StrafeManagerClass is %s"),
        StrafeManagerClass ? *StrafeManagerClass->GetName() : TEXT("NULL"));

    SpawnStrafeManager();

    AS_StrafeGameState* StrafeGS = GetStrafeGameState();
    if (CurrentStrafeManager && StrafeGS)
    {
        StrafeGS->SetStrafeManager(CurrentStrafeManager);
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode::StartPlay: StrafeManager set in GameState"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StrafeGameMode::StartPlay: Failed to set StrafeManager in GameState. Manager: %s, GameState: %s"),
            CurrentStrafeManager ? TEXT("Valid") : TEXT("NULL"),
            StrafeGS ? TEXT("Valid") : TEXT("NULL"));
    }
}

void AS_StrafeGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode::InitGame - Map: %s, Options: %s"), *MapName, *Options);
}

void AS_StrafeGameMode::InitGameState()
{
    Super::InitGameState();
    AS_StrafeGameState* StrafeGS = GetStrafeGameState();
    if (StrafeGS)
    {
        StrafeGS->MatchDurationSeconds = MatchDurationSeconds;
    }
}

void AS_StrafeGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: Player %s logged in."), *NewPlayer->GetName());

    // Add PlayerState type verification
    APlayerState* GenericPS = NewPlayer->GetPlayerState<APlayerState>();
    if (GenericPS)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode::PostLogin - PlayerController has PlayerState: %s of class %s"),
            *GenericPS->GetName(), *GenericPS->GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StrafeGameMode::PostLogin - PlayerController has NO PlayerState!"));
    }

    AS_StrafePlayerState* StrafePS = NewPlayer->GetPlayerState<AS_StrafePlayerState>();
    if (StrafePS)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode::PostLogin - Successfully cast to AS_StrafePlayerState"));
        if (CurrentStrafeManager)
        {
            CurrentStrafeManager->UpdatePlayerInScoreboard(StrafePS);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AS_StrafeGameMode::PostLogin: No StrafeManager available to update scoreboard!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StrafeGameMode::PostLogin - FAILED to cast PlayerState to AS_StrafePlayerState!"));
    }

    if (IsMatchInProgress() || GetMatchState() == MatchState::WaitingToStart || GetMatchState() == FName(TEXT("Warmup")))
    {
        RestartPlayer(NewPlayer);
    }
}

bool AS_StrafeGameMode::PlayerCanRestart(APlayerController* Player)
{
    if (GetMatchState() == MatchState::InProgress || GetMatchState() == FName(TEXT("Warmup")))
    {
        return true;
    }
    if (GetMatchState() == MatchState::WaitingToStart && WarmupTime <= 0.f)
    {
        return true;
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
    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: Match is WaitingToStart."));

    // Initialize the checkpoint system here
    if (CurrentStrafeManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: Initializing checkpoints via StrafeManager."));
        CurrentStrafeManager->RefreshAndInitializeCheckpoints();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StrafeGameMode: StrafeManager is NULL when trying to initialize checkpoints! Attempting to create one..."));

        // Try to spawn again as a fallback
        SpawnStrafeManager();

        if (CurrentStrafeManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: Successfully created StrafeManager on second attempt. Initializing checkpoints."));
            CurrentStrafeManager->RefreshAndInitializeCheckpoints();

            // Also update the GameState
            AS_StrafeGameState* StrafeGS = GetStrafeGameState();
            if (StrafeGS)
            {
                StrafeGS->SetStrafeManager(CurrentStrafeManager);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AS_StrafeGameMode: CRITICAL ERROR - Cannot create StrafeManager! Strafe mode will not function!"));
        }
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
    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: Warmup timer started for %f seconds."), WarmupTime);
}

void AS_StrafeGameMode::OnWarmupTimerEnd()
{
    GetWorldTimerManager().ClearTimer(WarmupTimerHandle);
    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: Warmup ended. Starting match."));
    StartMatch();
}

void AS_StrafeGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();
    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: Match has started (InProgress)."));

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
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: Main match timer started for %d seconds."), MatchDurationSeconds);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: No match duration set, match will run indefinitely (or until other conditions)."));
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
            UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: Match time limit reached."));
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
    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: Match has ended (WaitingPostMatch)."));
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
    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: Post-match timer started for %f seconds."), PostMatchTime);
}

void AS_StrafeGameMode::OnPostMatchTimerEnd()
{
    GetWorldTimerManager().ClearTimer(PostMatchTimerHandle);
    UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: Post-match ended."));
    RestartGame();
}

void AS_StrafeGameMode::SpawnStrafeManager()
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode::SpawnStrafeManager: Called on client. Ignoring."));
        return;
    }

    if (CurrentStrafeManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode::SpawnStrafeManager: StrafeManager already exists."));
        return;
    }

    if (!StrafeManagerClass)
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StrafeGameMode::SpawnStrafeManager: StrafeManagerClass is NULL! Using default AS_StrafeManager class."));
        StrafeManagerClass = AS_StrafeManager::StaticClass();
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    CurrentStrafeManager = GetWorld()->SpawnActor<AS_StrafeManager>(StrafeManagerClass, SpawnParams);

    if (CurrentStrafeManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_StrafeGameMode: Successfully spawned StrafeManager: %s"), *CurrentStrafeManager->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AS_StrafeGameMode: Failed to spawn StrafeManager from class %s!"), *StrafeManagerClass->GetName());
    }
}

AS_StrafeGameState* AS_StrafeGameMode::GetStrafeGameState() const
{
    return GetGameState<AS_StrafeGameState>();
}