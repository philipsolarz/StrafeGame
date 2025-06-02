#include "UI/ViewModels/S_KillfeedItemViewModel.h"
#include "GameFramework/PlayerState.h" 
#include "GameFramework/PlayerController.h" 

// FKillfeedEventData struct is defined in S_KillfeedItemViewModel.h

void US_KillfeedItemViewModel::Initialize(const FKillfeedEventData& KillData, APlayerController* LocalPlayerControllerContext)
{
    KillerName = KillData.KillerName;
    VictimName = KillData.VictimName;
    WeaponIcon = KillData.WeaponIcon;
    bIsLocalPlayerKiller = KillData.bIsLocalPlayerKiller;
    bIsLocalPlayerVictim = KillData.bIsLocalPlayerVictim;
    bWasSuicide = KillData.bWasSuicide; // Assignment for bWasSuicide

    // Logic to determine bIsLocalPlayerKiller/Victim should primarily happen when FKillfeedEventData is created
    // in AS_ArenaGameMode. If it's already correctly set in KillData, this re-check is redundant.
    if (LocalPlayerControllerContext && LocalPlayerControllerContext->PlayerState)
    {
        const FString LocalPlayerName = LocalPlayerControllerContext->PlayerState->GetPlayerName();
        if (!KillData.bIsLocalPlayerKiller && KillerName == LocalPlayerName)
        {
            bIsLocalPlayerKiller = true;
        }
        if (!KillData.bIsLocalPlayerVictim && VictimName == LocalPlayerName)
        {
            bIsLocalPlayerVictim = true;
        }
    }
}