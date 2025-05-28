// Source/StrafeGame/Private/GameModes/Arena/S_ArenaGameMode.cpp
#include "GameModes/Arena/S_ArenaGameMode.h"
#include "GameModes/Arena/S_ArenaGameState.h" 
#include "Player/S_PlayerController.h"
#include "Player/S_Character.h"
#include "GameModes/Arena/S_ArenaPlayerState.h" 
#include "TimerManager.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h" 

AS_ArenaGameMode::AS_ArenaGameMode()
{
    PlayerStateClass = AS_ArenaPlayerState::StaticClass();
    GameStateClass = AS_ArenaGameState::StaticClass();

    MatchTimeLimitSeconds = 300;
    FragLimit = 20;
    WarmupTime = 15.0f;
    PostMatchTime = 20.0f;
}

void AS_ArenaGameMode::InitGameState()
{
    Super::InitGameState();
    AS_ArenaGameState* ArenaGS = GetGameState<AS_ArenaGameState>();
    if (ArenaGS)
    {
        ArenaGS->FragLimit = FragLimit;
        ArenaGS->MatchDurationSeconds = MatchTimeLimitSeconds; // Also replicate MatchTimeLimit
    }
}

void AS_ArenaGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Player %s logged in."), *NewPlayer->GetName());

    // if (IsMatchInProgress() || GetMatchState() == MatchState::WaitingToStart || GetMatchState() == MatchState::Warmup) // Old
    if (IsMatchInProgress() || GetMatchState() == MatchState::WaitingToStart || GetMatchState() == FName(TEXT("Warmup"))) // Corrected
    {
        RestartPlayer(NewPlayer);
    }
}

void AS_ArenaGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    CheckMatchEndConditions();
}

void AS_ArenaGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    if (GetMatchState() == MatchState::WaitingPostMatch || GetMatchState() == MatchState::LeavingMap)
    {
        ErrorMessage = TEXT("Match has already ended.");
    }
}

bool AS_ArenaGameMode::PlayerCanRestart(APlayerController* Player)
{
    // if (GetMatchState() == MatchState::InProgress || GetMatchState() == MatchState::Warmup) // Old
    if (GetMatchState() == MatchState::InProgress || GetMatchState() == FName(TEXT("Warmup"))) // Corrected
    {
        return Super::PlayerCanRestart(Player);
    }
    return false;
}

void AS_ArenaGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}


void AS_ArenaGameMode::HandleMatchIsWaitingToStart()
{
    Super::HandleMatchIsWaitingToStart();
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Match is WaitingToStart. Proceeding to Warmup."));

    if (WarmupTime > 0.f)
    {
        SetMatchState(FName(TEXT("Warmup"))); // Corrected: Set actual MatchState FName
        StartWarmupTimer();
        AS_ArenaGameState* ArenaGS = GetArenaGameState();
        if (ArenaGS)
        {
            ArenaGS->SetRemainingTime(FMath::CeilToInt(WarmupTime));
            // ArenaGS->SetMatchStateNameOverride(FName(TEXT("Warmup"))); // This can still be used if MatchState FName itself isn't enough for UI
        }
    }
    else
    {
        StartMatch();
    }
}

void AS_ArenaGameMode::StartWarmupTimer()
{
    GetWorldTimerManager().SetTimer(WarmupTimerHandle, this, &AS_ArenaGameMode::OnWarmupTimerEnd, WarmupTime, false);
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Warmup timer started for %f seconds."), WarmupTime);
    AS_ArenaGameState* ArenaGS = GetArenaGameState();
    if (ArenaGS) ArenaGS->SetMatchStateNameOverride(FName(TEXT("Warmup")));
}

void AS_ArenaGameMode::OnWarmupTimerEnd()
{
    GetWorldTimerManager().ClearTimer(WarmupTimerHandle);
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Warmup ended. Starting match."));
    StartMatch();
}


void AS_ArenaGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Match has started (InProgress)."));

    AS_ArenaGameState* ArenaGS = GetArenaGameState();
    if (ArenaGS)
    {
        ArenaGS->SetRemainingTime(MatchTimeLimitSeconds);
        ArenaGS->SetMatchStateNameOverride(NAME_None);
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->GetPawn() == nullptr)
        {
            RestartPlayer(PC);
        }
    }
    StartMainMatchTimer();
}

void AS_ArenaGameMode::StartMainMatchTimer()
{
    if (MatchTimeLimitSeconds > 0)
    {
        GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &AS_ArenaGameMode::UpdateMainMatchTime, 1.0f, true);
        UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Main match timer started for %d seconds."), MatchTimeLimitSeconds);
    }
}

void AS_ArenaGameMode::UpdateMainMatchTime()
{
    AS_ArenaGameState* ArenaGS = GetArenaGameState();
    if (ArenaGS && IsMatchInProgress())
    {
        int32 CurrentTime = ArenaGS->RemainingTime;
        CurrentTime--;
        ArenaGS->SetRemainingTime(CurrentTime);

        if (CurrentTime <= 0)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Match time limit reached."));
            GetWorldTimerManager().ClearTimer(MatchTimerHandle);
            EndMatch();
        }
    }
    else if (IsMatchInProgress())
    {
        GetWorldTimerManager().ClearTimer(MatchTimerHandle);
    }
}

void AS_ArenaGameMode::HandleMatchHasEnded()
{
    Super::HandleMatchHasEnded();
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Match has ended (WaitingPostMatch)."));

    GetWorldTimerManager().ClearTimer(MatchTimerHandle);

    AS_ArenaPlayerState* Winner = GetMatchWinner();
    FName Reason = (FragLimit > 0 && Winner && Winner->GetFrags() >= FragLimit) ? FName(TEXT("FragLimitReached")) : FName(TEXT("TimeLimitReached"));

    OnArenaMatchEndedDelegate_Native.Broadcast(Winner, Reason);

    AS_ArenaGameState* ArenaGS = GetArenaGameState(); // Get it again for safety
    if (ArenaGS)
    {
        ArenaGS->SetMatchStateNameOverride(FName(TEXT("PostMatch"))); // Set custom state for UI
    }


    if (PostMatchTime > 0.f)
    {
        StartPostMatchTimer();
        if (ArenaGS) ArenaGS->SetRemainingTime(FMath::CeilToInt(PostMatchTime));
    }
    else
    {
        OnPostMatchTimerEnd();
    }
}

void AS_ArenaGameMode::StartPostMatchTimer()
{
    GetWorldTimerManager().SetTimer(PostMatchTimerHandle, this, &AS_ArenaGameMode::OnPostMatchTimerEnd, PostMatchTime, false);
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Post-match timer started for %f seconds."), PostMatchTime);
}

void AS_ArenaGameMode::OnPostMatchTimerEnd()
{
    GetWorldTimerManager().ClearTimer(PostMatchTimerHandle);
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Post-match ended. Restarting game or returning to lobby."));
    RestartGame();
}


void AS_ArenaGameMode::OnPlayerKilled(AS_PlayerState* VictimBasePS, AS_PlayerState* KillerBasePS, AActor* KillingDamageCauser, AController* VictimController, AController* KillerController)
{
    Super::OnPlayerKilled(VictimBasePS, KillerBasePS, KillingDamageCauser, VictimController, KillerController);

    AS_ArenaPlayerState* VictimPlayerState = Cast<AS_ArenaPlayerState>(VictimBasePS);
    AS_ArenaPlayerState* KillerPlayerState = Cast<AS_ArenaPlayerState>(KillerBasePS);

    if (VictimPlayerState)
    {
        VictimPlayerState->ScoreDeath(VictimPlayerState, KillerPlayerState);
    }

    if (KillerPlayerState)
    {
        if (KillerPlayerState != VictimPlayerState)
        {
            KillerPlayerState->ScoreFrag(KillerPlayerState, VictimPlayerState);
        }
    }
    CheckMatchEndConditions();
}

void AS_ArenaGameMode::CheckMatchEndConditions()
{
    if (!IsMatchInProgress()) return;

    if (FragLimit > 0)
    {
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            if (PC)
            {
                AS_ArenaPlayerState* PS = PC->GetPlayerState<AS_ArenaPlayerState>();
                if (PS && PS->GetFrags() >= FragLimit)
                {
                    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Player %s reached frag limit (%d). Ending match."), *PS->GetPlayerName(), FragLimit);
                    EndMatch();
                    return;
                }
            }
        }
    }
}

AS_ArenaPlayerState* AS_ArenaGameMode::GetMatchWinner() const
{
    AS_ArenaPlayerState* Winner = nullptr;
    int32 MaxFrags = -1;
    bool bIsDraw = false;

    if (GameState)
    {
        for (APlayerState* PS : GameState->PlayerArray)
        {
            AS_ArenaPlayerState* ArenaPS = Cast<AS_ArenaPlayerState>(PS);
            if (ArenaPS)
            {
                if (ArenaPS->GetFrags() > MaxFrags)
                {
                    MaxFrags = ArenaPS->GetFrags();
                    Winner = ArenaPS;
                    bIsDraw = false;
                }
                else if (ArenaPS->GetFrags() == MaxFrags && MaxFrags != -1)
                {
                    Winner = nullptr; // Nullify winner in case of a draw
                    bIsDraw = true;
                }
            }
        }
    }
    return Winner; // If bIsDraw was true, Winner will be nullptr
}

AS_ArenaGameState* AS_ArenaGameMode::GetArenaGameState() const
{
    return GetGameState<AS_ArenaGameState>();
}