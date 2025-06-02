// Source/StrafeGame/Private/UI/ViewModels/S_GameModeHUDViewModelBase.cpp
#include "UI/ViewModels/S_GameModeHUDViewModelBase.h"
#include "Player/S_PlayerController.h"
#include "GameModes/S_GameStateBase.h"
#include "Engine/World.h" // For GetWorld()

void US_GameModeHUDViewModelBase::Initialize(AS_PlayerController* InOwningPlayerController)
{
    Super::Initialize(InOwningPlayerController);

    if (GetOwningPlayerController() && GetOwningPlayerController()->GetWorld())
    {
        GameStateBase = GetOwningPlayerController()->GetWorld()->GetGameState<AS_GameStateBase>();
        if (GameStateBase.IsValid())
        {
            // Assuming AS_GameStateBase has these delegates, or we'd poll/use OnReps
            // If MatchStateChangedDelegate exists and is dynamic:
            // GameStateBase->OnMatchStateChangedDelegate.AddDynamic(this, &US_GameModeHUDViewModelBase::HandleMatchStateChanged);

            // Corrected binding for dynamic delegate
            GameStateBase->OnRemainingTimeChangedDelegate.AddDynamic(this, &US_GameModeHUDViewModelBase::HandleRemainingTimeChanged);

            RefreshGameModeData();
        }
    }
}

void US_GameModeHUDViewModelBase::Deinitialize()
{
    if (GameStateBase.IsValid())
    {
        // If MatchStateChangedDelegate exists and is dynamic:
        // GameStateBase->OnMatchStateChangedDelegate.RemoveDynamic(this, &US_GameModeHUDViewModelBase::HandleMatchStateChanged);

        // Corrected unbinding for dynamic delegate
        GameStateBase->OnRemainingTimeChangedDelegate.RemoveDynamic(this, &US_GameModeHUDViewModelBase::HandleRemainingTimeChanged);
    }
    GameStateBase.Reset();
    Super::Deinitialize();
}

void US_GameModeHUDViewModelBase::RefreshGameModeData()
{
    UpdateMatchStateName();
    UpdateRemainingTime();
    OnGameModeViewModelUpdated.Broadcast();
}

void US_GameModeHUDViewModelBase::UpdateMatchStateName()
{
    if (GameStateBase.IsValid())
    {
        CurrentMatchStateName = GameStateBase->GetMatchStateName();
    }
    else
    {
        CurrentMatchStateName = NAME_None;
    }
}

void US_GameModeHUDViewModelBase::UpdateRemainingTime()
{
    if (GameStateBase.IsValid())
    {
        RemainingMatchTimeSeconds = GameStateBase->RemainingTime;
    }
    else
    {
        RemainingMatchTimeSeconds = 0;
    }
}

void US_GameModeHUDViewModelBase::HandleMatchStateChanged(FName NewMatchState)
{
    CurrentMatchStateName = NewMatchState;
    OnGameModeViewModelUpdated.Broadcast();
}

void US_GameModeHUDViewModelBase::HandleRemainingTimeChanged(int32 NewTime)
{
    RemainingMatchTimeSeconds = NewTime;
    OnGameModeViewModelUpdated.Broadcast();
}