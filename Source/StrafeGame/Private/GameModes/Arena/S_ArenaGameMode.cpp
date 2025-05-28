#include "GameModes/Arena/S_ArenaGameMode.h"
#include "GameModes/Arena/S_ArenaGameState.h" // Specific GameState
#include "Player/S_PlayerController.h"
#include "Player/S_Character.h"
#include "Player/GameModes/Arena/S_ArenaPlayerState.h" // Specific PlayerState
#include "TimerManager.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h" // For setting default classes in constructor

AS_ArenaGameMode::AS_ArenaGameMode()
{
    // Set default classes specific to Arena mode
    PlayerStateClass = AS_ArenaPlayerState::StaticClass();
    GameStateClass = AS_ArenaGameState::StaticClass();
    // DefaultPawnClass and PlayerControllerClass can be inherited from AS_GameModeBase if they are the same.

    MatchTimeLimitSeconds = 300; // 5 minutes
    FragLimit = 20;
    WarmupTime = 15.0f; // 15 seconds warmup
    PostMatchTime = 20.0f; // 20 seconds post-match scoreboard display
}

void AS_ArenaGameMode::InitGameState()
{
    Super::InitGameState();
    AS_ArenaGameState* ArenaGS = GetGameState<AS_ArenaGameState>();
    if (ArenaGS)
    {
        ArenaGS->FragLimit = FragLimit; // Initialize replicated frag limit on GameState
        // MatchTimeLimitSeconds will be used to set RemainingTime when match starts
    }
}

void AS_ArenaGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer); // Calls InitializePlayer
    // Arena specific PostLogin logic, e.g., assign to a default team if FFA has "teams" for display.
    // For true FFA, not much else is needed here beyond base class.
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Player %s logged in."), *NewPlayer->GetName());

    // If match is in warmup or already started, spawn pawn immediately
    if (IsMatchInProgress() || GetMatchState() == MatchState::WaitingToStart || GetMatchState() == MatchState::Warmup)
    {
        RestartPlayer(NewPlayer);
    }
}

void AS_ArenaGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    // Arena specific logout logic, e.g., update scoreboard calculations if a player leaves mid-game.
    CheckMatchEndConditions(); // A player leaving might trigger a win condition if few players remain
}

void AS_ArenaGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    // Example: Don't allow players to join if match is in PostMatch state or about to end
    // if (GetMatchState() == MatchState::WaitingPostMatch || GetMatchState() == MatchState::LeavingMap)
    // {
    //     ErrorMessage = TEXT("Match has already ended.");
    // }
}

bool AS_ArenaGameMode::PlayerCanRestart(APlayerController* Player)
{
    // Allow restart if match is in progress or warmup.
    // Disallow if match hasn't started or is over.
    if (GetMatchState() == MatchState::InProgress || GetMatchState() == MatchState::Warmup)
    {
        return Super::PlayerCanRestart(Player); // Base class checks if player is dead or spectator
    }
    return false;
}

void AS_ArenaGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // AGameMode::Tick handles match state transitions timer automatically.
    // We use our own timers for more explicit control over warmup/main play/post-match durations.
}


void AS_ArenaGameMode::HandleMatchIsWaitingToStart()
{
    Super::HandleMatchIsWaitingToStart();
    // This state is typically before warmup. Players connect, but game hasn't "started" countdowns.
    // You could hold players here until a certain number connect, or just proceed to warmup.
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Match is WaitingToStart. Proceeding to Warmup."));

    if (WarmupTime > 0.f)
    {
        SetMatchState(MatchState::Warmup); // Custom state if AGameMode doesn't have it, or use WaitingToStart
        // AGameMode uses "WaitingToStart" then ReadyToStartMatch() -> StartMatch()
        // For simplicity, let's assume we go from WaitingToStart to InProgress via StartMatch()
        // and HandleMatchHasStarted will manage the warmup timer.
        // Or, we can use a custom flow: WaitingToStart -> Warmup -> InProgress
        StartWarmupTimer();
        AS_ArenaGameState* ArenaGS = GetArenaGameState();
        if (ArenaGS) ArenaGS->SetRemainingTime(FMath::CeilToInt(WarmupTime));
    }
    else
    {
        // No warmup, try to start the match directly
        StartMatch(); // This will call HandleMatchHasStarted
    }
}

void AS_ArenaGameMode::StartWarmupTimer()
{
    GetWorldTimerManager().SetTimer(WarmupTimerHandle, this, &AS_ArenaGameMode::OnWarmupTimerEnd, WarmupTime, false);
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Warmup timer started for %f seconds."), WarmupTime);
    // GameState should reflect warmup state
    AS_ArenaGameState* ArenaGS = GetArenaGameState();
    if (ArenaGS) ArenaGS->SetMatchStateNameOverride(FName(TEXT("Warmup"))); // Custom state name if needed for UI
}

void AS_ArenaGameMode::OnWarmupTimerEnd()
{
    GetWorldTimerManager().ClearTimer(WarmupTimerHandle);
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Warmup ended. Starting match."));
    StartMatch(); // This will transition to InProgress and call HandleMatchHasStarted
}


void AS_ArenaGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted(); // This sets MatchState to InProgress
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Match has started (InProgress)."));

    // Initialize GameState for the match
    AS_ArenaGameState* ArenaGS = GetArenaGameState();
    if (ArenaGS)
    {
        ArenaGS->SetRemainingTime(MatchTimeLimitSeconds);
        // ArenaGS->SetMatchStateNameOverride(NAME_None); // Clear override if we set one for warmup
    }

    // Spawn all players who are already connected
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->GetPawn() == nullptr) // If player has no pawn
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
    if (ArenaGS && IsMatchInProgress()) // IsMatchInProgress checks GameMode's MatchState
    {
        int32 CurrentTime = ArenaGS->RemainingTime;
        CurrentTime--;
        ArenaGS->SetRemainingTime(CurrentTime); // Replicates to clients

        if (CurrentTime <= 0)
        {
            UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Match time limit reached."));
            GetWorldTimerManager().ClearTimer(MatchTimerHandle);
            EndMatch(); // AGameMode's EndMatch will set state to WaitingPostMatch
            // We'll determine winner in HandleMatchHasEnded or a specific EndMatch override
        }
    }
    else if (IsMatchInProgress()) // Should not happen if ArenaGS is null, but as a fallback
    {
        GetWorldTimerManager().ClearTimer(MatchTimerHandle);
    }
}

void AS_ArenaGameMode::HandleMatchHasEnded()
{
    // This is called by AGameMode::EndMatch()
    Super::HandleMatchHasEnded(); // Sets MatchState to WaitingPostMatch
    UE_LOG(LogTemp, Log, TEXT("AS_ArenaGameMode: Match has ended (WaitingPostMatch)."));

    GetWorldTimerManager().ClearTimer(MatchTimerHandle); // Ensure main timer is stopped

    AS_ArenaPlayerState* Winner = GetMatchWinner();
    FName Reason = (FragLimit > 0 && Winner && Winner->GetFrags() >= FragLimit) ? FName(TEXT("FragLimitReached")) : FName(TEXT("TimeLimitReached"));

    OnArenaMatchEndedDelegate_Native.Broadcast(Winner, Reason);

    // Start post-match timer
    if (PostMatchTime > 0.f)
    {
        StartPostMatchTimer();
        AS_ArenaGameState* ArenaGS = GetArenaGameState();
        if (ArenaGS) ArenaGS->SetRemainingTime(FMath::CeilToInt(PostMatchTime));
    }
    else
    {
        // No post-match time, potentially restart or go to lobby immediately
        // RestartGame(); // Example
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
    RestartGame(); // AGameModeBase function, typically travels to the current map again.
    // Or implement custom logic like returning to a lobby map.
}


void AS_ArenaGameMode::OnPlayerKilled(AS_PlayerState* VictimBasePS, AS_PlayerState* KillerBasePS, AActor* KillingDamageCauser, AController* VictimController, AController* KillerController)
{
    Super::OnPlayerKilled(VictimBasePS, KillerBasePS, KillingDamageCauser, VictimController, KillerController); // Handles basic respawn request

    AS_ArenaPlayerState* VictimPlayerState = Cast<AS_ArenaPlayerState>(VictimBasePS);
    AS_ArenaPlayerState* KillerPlayerState = Cast<AS_ArenaPlayerState>(KillerBasePS);

    if (VictimPlayerState)
    {
        VictimPlayerState->ScoreDeath(VictimPlayerState, KillerPlayerState);
    }

    if (KillerPlayerState)
    {
        // KillerPlayerState != VictimPlayerState ensures no score for self-kill/suicide
        if (KillerPlayerState != VictimPlayerState)
        {
            KillerPlayerState->ScoreFrag(KillerPlayerState, VictimPlayerState);
        }
    }
    CheckMatchEndConditions();
}

void AS_ArenaGameMode::CheckMatchEndConditions()
{
    if (!IsMatchInProgress()) return; // Only check if match is running

    // Check Frag Limit
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
                    EndMatch(); // AGameMode's EndMatch sets state to WaitingPostMatch. Winner determined in HandleMatchHasEnded.
                    return;
                }
            }
        }
    }
    // Time limit is handled by UpdateMainMatchTime directly calling EndMatch.
}

AS_ArenaPlayerState* AS_ArenaGameMode::GetMatchWinner() const
{
    AS_ArenaPlayerState* Winner = nullptr;
    int32 MaxFrags = -1; // Start with -1 to ensure first player with >= 0 frags can become winner
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
                else if (ArenaPS->GetFrags() == MaxFrags && MaxFrags != -1) // Draw if multiple players have same high score
                {
                    bIsDraw = true;
                }
            }
        }
    }
    return bIsDraw ? nullptr : Winner;
}

AS_ArenaGameState* AS_ArenaGameMode::GetArenaGameState() const
{
    return GetGameState<AS_ArenaGameState>();
}