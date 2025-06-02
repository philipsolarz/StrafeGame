// Source/StrafeGame/Private/UI/ViewModels/S_ArenaHUDViewModel.cpp
#include "UI/ViewModels/S_ArenaHUDViewModel.h"
#include "Player/S_PlayerController.h"
#include "Player/S_PlayerState.h" 
#include "GameModes/Arena/S_ArenaGameState.h" // For AS_ArenaGameState and FKillfeedEventData (if struct moved here)
#include "GameModes/Arena/S_ArenaPlayerState.h"
#include "UI/ViewModels/S_KillfeedItemViewModel.h" // Include the item ViewModel
#include "Engine/World.h"
#include "EngineUtils.h" 

void US_ArenaHUDViewModel::Initialize(AS_PlayerController* InOwningPlayerController)
{
    Super::Initialize(InOwningPlayerController);

    if (GetOwningPlayerController() && GetOwningPlayerController()->GetWorld())
    {
        ArenaGameState = Cast<AS_ArenaGameState>(GameStateBase.Get());
        LocalArenaPlayerState = GetOwningPlayerController()->GetPlayerState<AS_ArenaPlayerState>();

        if (LocalArenaPlayerState.IsValid())
        {
            LocalArenaPlayerState->OnArenaScoreUpdatedDelegate.AddDynamic(this, &US_ArenaHUDViewModel::HandleLocalPlayerScoreUpdated);
        }

        if (ArenaGameState.IsValid())
        {
            // Subscribe to the GameState's killfeed update delegate
            ArenaGameState->OnKillfeedUpdated.AddDynamic(this, &US_ArenaHUDViewModel::HandleGameStateKillfeedUpdated);
        }

        RefreshGameModeData();
    }
}

void US_ArenaHUDViewModel::Deinitialize()
{
    if (LocalArenaPlayerState.IsValid())
    {
        LocalArenaPlayerState->OnArenaScoreUpdatedDelegate.RemoveDynamic(this, &US_ArenaHUDViewModel::HandleLocalPlayerScoreUpdated);
    }
    if (ArenaGameState.IsValid())
    {
        ArenaGameState->OnKillfeedUpdated.RemoveDynamic(this, &US_ArenaHUDViewModel::HandleGameStateKillfeedUpdated);
    }

    // Clear ViewModels to prevent dangling pointers if they were created with this as outer
    for (TObjectPtr<US_KillfeedItemViewModel> KillVM : KillfeedItemViewModels)
    {
        if (KillVM)
        {
            // If ViewModels were NewObject<US_KillfeedItemViewModel>(this), they'd be GC'd with this.
            // If they were NewObject<US_KillfeedItemViewModel>() (no outer), they need to be managed or will leak.
            // Assuming they are outered to this for simplicity or GC will handle if no strong refs.
        }
    }
    KillfeedItemViewModels.Empty();

    ArenaGameState.Reset();
    LocalArenaPlayerState.Reset();
    Super::Deinitialize();
}

void US_ArenaHUDViewModel::RefreshGameModeData()
{
    Super::RefreshGameModeData();
    UpdateArenaSpecificData();
    UpdateKillfeed(); // Update killfeed as part of general refresh
    OnGameModeViewModelUpdated.Broadcast();
}

void US_ArenaHUDViewModel::UpdateArenaSpecificData()
{
    if (ArenaGameState.IsValid())
    {
        FragLimit = ArenaGameState->FragLimit;
        LeaderName = TEXT("Top Player");
        LeaderFrags = 0;

        if (GetWorld())
        {
            int32 MaxFrags = -1;
            AS_ArenaPlayerState* CurrentLeader = nullptr;
            for (TActorIterator<APlayerState> It(GetWorld()); It; ++It)
            {
                AS_ArenaPlayerState* PS = Cast<AS_ArenaPlayerState>(*It);
                if (PS)
                {
                    if (PS->GetFrags() > MaxFrags)
                    {
                        MaxFrags = PS->GetFrags();
                        CurrentLeader = PS;
                    }
                    else if (PS->GetFrags() == MaxFrags && MaxFrags != -1)
                    {
                        CurrentLeader = nullptr;
                    }
                }
            }
            if (CurrentLeader)
            {
                LeaderName = CurrentLeader->GetPlayerName();
                LeaderFrags = CurrentLeader->GetFrags();
            }
            else if (MaxFrags != -1)
            {
                LeaderName = TEXT("DRAW");
                LeaderFrags = MaxFrags;
            }
            else
            {
                LeaderName = TEXT("-");
                LeaderFrags = 0;
            }
        }
    }
    else
    {
        FragLimit = 0;
        LeaderName = TEXT("N/A");
        LeaderFrags = 0;
    }

    if (LocalArenaPlayerState.IsValid())
    {
        PlayerFrags = LocalArenaPlayerState->GetFrags();
        PlayerDeaths = LocalArenaPlayerState->GetDeaths();
    }
    else
    {
        PlayerFrags = 0;
        PlayerDeaths = 0;
    }
}

void US_ArenaHUDViewModel::UpdateKillfeed()
{
    KillfeedItemViewModels.Empty();

    if (ArenaGameState.IsValid() && GetOwningPlayerController())
    {
        // This loop now expects ArenaGameState->RecentKillsLog to be TArray<FKillfeedEventData>
        for (const FKillfeedEventData& KillData : ArenaGameState->RecentKillsLog)
        {
            US_KillfeedItemViewModel* KillVM = NewObject<US_KillfeedItemViewModel>(this);
            if (KillVM)
            {
                // Initialize now expects FKillfeedEventData, which KillData will be after AS_ArenaGameState.h is fixed
                KillVM->Initialize(KillData, GetOwningPlayerController());
                KillfeedItemViewModels.Add(KillVM);
            }
        }
    }
}


void US_ArenaHUDViewModel::HandleLocalPlayerScoreUpdated(AS_ArenaPlayerState* InPlayerState, int32 NewFrags, int32 NewDeaths)
{
    if (InPlayerState == LocalArenaPlayerState.Get())
    {
        PlayerFrags = NewFrags;
        PlayerDeaths = NewDeaths;
        UpdateArenaSpecificData();
        OnGameModeViewModelUpdated.Broadcast();
    }
}

void US_ArenaHUDViewModel::HandleGameStateKillfeedUpdated()
{
    UpdateKillfeed();
    OnGameModeViewModelUpdated.Broadcast(); // Notify the View (WBP_ArenaStatusWidget)
}