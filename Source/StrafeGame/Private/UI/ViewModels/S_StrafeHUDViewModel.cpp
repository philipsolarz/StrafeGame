#include "UI/ViewModels/S_StrafeHUDViewModel.h"
#include "Player/S_PlayerController.h"
#include "GameModes/Strafe/S_StrafeGameState.h"
#include "GameModes/Strafe/S_StrafePlayerState.h"
#include "GameModes/Strafe/S_StrafeManager.h"
#include "Engine/World.h"

void US_StrafeHUDViewModel::Initialize(AS_PlayerController* InOwningPlayerController)
{
    Super::Initialize(InOwningPlayerController);

    UE_LOG(LogTemp, Warning, TEXT("US_StrafeHUDViewModel::Initialize called for PC: %s"),
        InOwningPlayerController ? *InOwningPlayerController->GetName() : TEXT("NULL"));

    if (GetOwningPlayerController() && GetOwningPlayerController()->GetWorld())
    {
        StrafeGameState = Cast<AS_StrafeGameState>(GameStateBase.Get());
        LocalStrafePlayerState = GetOwningPlayerController()->GetPlayerState<AS_StrafePlayerState>();

        if (LocalStrafePlayerState.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("US_StrafeHUDViewModel::Initialize - Found StrafePlayerState: %s"),
                *LocalStrafePlayerState->GetName());

            LocalStrafePlayerState->OnStrafePlayerRaceStateChangedDelegate.AddDynamic(this, &US_StrafeHUDViewModel::HandleStrafeRaceStateChanged);

            UE_LOG(LogTemp, Warning, TEXT("US_StrafeHUDViewModel::Initialize - Bound to race state change delegate"));

            // Start a timer to update the race time continuously
            if (UWorld* World = GetOwningPlayerController()->GetWorld())
            {
                World->GetTimerManager().SetTimer(UpdateTimerHandle, this, &US_StrafeHUDViewModel::UpdateRaceTime, 0.05f, true);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("US_StrafeHUDViewModel::Initialize - No StrafePlayerState found!"));
        }

        RefreshGameModeData();
    }
}

// Add this new function:
void US_StrafeHUDViewModel::UpdateRaceTime()
{
    if (LocalStrafePlayerState.IsValid() && LocalStrafePlayerState->IsRaceInProgress())
    {
        float NewTime = LocalStrafePlayerState->GetCurrentRaceTime();
        if (!FMath::IsNearlyEqual(CurrentRaceTime, NewTime, 0.01f))
        {
            CurrentRaceTime = NewTime;
            OnGameModeViewModelUpdated.Broadcast();
        }
    }
}

// Update Deinitialize to clear the timer:
void US_StrafeHUDViewModel::Deinitialize()
{
    if (GetOwningPlayerController() && GetOwningPlayerController()->GetWorld())
    {
        GetOwningPlayerController()->GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    if (LocalStrafePlayerState.IsValid())
    {
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

    UE_LOG(LogTemp, Warning, TEXT("US_StrafeHUDViewModel::RefreshGameModeData - Broadcasting update. CurrentTime: %.2f, Active: %s"),
        CurrentRaceTime, bIsRaceActive ? TEXT("YES") : TEXT("NO"));
}

void US_StrafeHUDViewModel::UpdateStrafeSpecificData()
{
    if (LocalStrafePlayerState.IsValid())
    {
        CurrentRaceTime = LocalStrafePlayerState->GetCurrentRaceTime();
        BestRaceTime = LocalStrafePlayerState->GetBestRaceTime();
        CurrentSplitTimes = LocalStrafePlayerState->GetCurrentSplitTimes();
        SplitDeltas = LocalStrafePlayerState->GetCurrentSplitDeltas();
        CurrentCheckpoint = LocalStrafePlayerState->GetLastCheckpointReached();
        bIsRaceActive = LocalStrafePlayerState->IsRaceInProgress();

        UE_LOG(LogTemp, Warning, TEXT("US_StrafeHUDViewModel::UpdateStrafeSpecificData - Time: %.2f, Checkpoint: %d, Active: %s"),
            CurrentRaceTime, CurrentCheckpoint, bIsRaceActive ? TEXT("YES") : TEXT("NO"));
    }
    else
    {
        CurrentRaceTime = 0.0f;
        BestRaceTime.Reset();
        CurrentSplitTimes.Empty();
        SplitDeltas.Empty();
        CurrentCheckpoint = -1;
        bIsRaceActive = false;

        UE_LOG(LogTemp, Warning, TEXT("US_StrafeHUDViewModel::UpdateStrafeSpecificData - No PlayerState, resetting values"));
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
    UE_LOG(LogTemp, Warning, TEXT("US_StrafeHUDViewModel::HandleStrafeRaceStateChanged called for PlayerState: %s"),
        InPlayerState ? *InPlayerState->GetName() : TEXT("NULL"));

    if (InPlayerState == LocalStrafePlayerState.Get())
    {
        UpdateStrafeSpecificData();
        OnGameModeViewModelUpdated.Broadcast();

        UE_LOG(LogTemp, Warning, TEXT("US_StrafeHUDViewModel::HandleStrafeRaceStateChanged - Updated and broadcast"));
    }
}