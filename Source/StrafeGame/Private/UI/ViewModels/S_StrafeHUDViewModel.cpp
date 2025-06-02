// Source/StrafeGame/Private/UI/ViewModels/S_StrafeHUDViewModel.cpp
#include "UI/ViewModels/S_StrafeHUDViewModel.h"
#include "Player/S_PlayerController.h"
#include "GameModes/Strafe/S_StrafeGameState.h"
#include "GameModes/Strafe/S_StrafePlayerState.h"
#include "GameModes/Strafe/S_StrafeManager.h"
#include "Engine/World.h"

void US_StrafeHUDViewModel::Initialize(AS_PlayerController* InOwningPlayerController)
{
    Super::Initialize(InOwningPlayerController);

    if (GetOwningPlayerController() && GetOwningPlayerController()->GetWorld())
    {
        StrafeGameState = Cast<AS_StrafeGameState>(GameStateBase.Get());
        LocalStrafePlayerState = GetOwningPlayerController()->GetPlayerState<AS_StrafePlayerState>();

        if (LocalStrafePlayerState.IsValid())
        {
            // Corrected binding
            LocalStrafePlayerState->OnStrafePlayerRaceStateChangedDelegate.AddDynamic(this, &US_StrafeHUDViewModel::HandleStrafeRaceStateChanged);
        }
        RefreshGameModeData();
    }
}

void US_StrafeHUDViewModel::Deinitialize()
{
    if (LocalStrafePlayerState.IsValid())
    {
        // Corrected unbinding
        LocalStrafePlayerState->OnStrafePlayerRaceStateChangedDelegate.RemoveDynamic(this, &US_StrafeHUDViewModel::HandleStrafeRaceStateChanged);
    }
    StrafeGameState.Reset();
    LocalStrafePlayerState.Reset();
    Super::Deinitialize();
}

void US_StrafeHUDViewModel::RefreshGameModeData()
{
    Super::RefreshGameModeData();
    UpdateStrafeSpecificData();
    OnGameModeViewModelUpdated.Broadcast();
}

void US_StrafeHUDViewModel::UpdateStrafeSpecificData()
{
    if (LocalStrafePlayerState.IsValid())
    {
        CurrentRaceTime = LocalStrafePlayerState->GetCurrentRaceTime();
        BestRaceTime = LocalStrafePlayerState->GetBestRaceTime();
        CurrentSplitTimes = LocalStrafePlayerState->GetCurrentSplitTimes();
        CurrentCheckpoint = LocalStrafePlayerState->GetLastCheckpointReached();
        bIsRaceActive = LocalStrafePlayerState->IsRaceInProgress();
    }
    else
    {
        CurrentRaceTime = 0.0f;
        BestRaceTime.Reset();
        CurrentSplitTimes.Empty();
        CurrentCheckpoint = -1;
        bIsRaceActive = false;
    }

    if (StrafeGameState.IsValid() && StrafeGameState->StrafeManager)
    {
        TotalCheckpoints = StrafeGameState->StrafeManager->GetTotalCheckpointsForLap();
    }
    else
    {
        TotalCheckpoints = 0;
    }
}

void US_StrafeHUDViewModel::HandleStrafeRaceStateChanged(AS_StrafePlayerState* InPlayerState)
{
    if (InPlayerState == LocalStrafePlayerState.Get())
    {
        UpdateStrafeSpecificData();
        OnGameModeViewModelUpdated.Broadcast();
    }
}