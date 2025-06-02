// Source/StrafeGame/Private/UI/ViewModels/S_ChargedShotgunViewModel.cpp
#include "UI/ViewModels/S_ChargedShotgunViewModel.h"
#include "Weapons/ChargedShotgun/S_ChargedShotgun.h"
#include "Player/S_PlayerController.h"
// Removed TimerManager and Engine/World as they are not needed for delegate approach

void US_ChargedShotgunViewModel::Initialize(AS_PlayerController* InOwningPlayerController, AS_Weapon* InWeaponActor)
{
    Super::Initialize(InOwningPlayerController, InWeaponActor); // Calls base US_WeaponViewModel::Initialize
    ChargedShotgunActor = Cast<AS_ChargedShotgun>(WeaponActor.Get()); // Get from base after it's set

    PrimaryChargeProgress = 0.0f;
    SecondaryChargeProgress = 0.0f;

    if (ChargedShotgunActor.IsValid())
    {
        ChargedShotgunActor->OnPrimaryChargeProgressChanged.AddDynamic(this, &US_ChargedShotgunViewModel::HandlePrimaryChargeProgressChanged);
        ChargedShotgunActor->OnSecondaryChargeProgressChanged.AddDynamic(this, &US_ChargedShotgunViewModel::HandleSecondaryChargeProgressChanged);

        // Initial update
        PrimaryChargeProgress = ChargedShotgunActor->GetPrimaryChargeProgress();
        SecondaryChargeProgress = ChargedShotgunActor->GetSecondaryChargeProgress();
        OnWeaponViewModelUpdated.Broadcast(); // From base class, signals generic update
    }
}

void US_ChargedShotgunViewModel::Deinitialize()
{
    if (ChargedShotgunActor.IsValid())
    {
        ChargedShotgunActor->OnPrimaryChargeProgressChanged.RemoveDynamic(this, &US_ChargedShotgunViewModel::HandlePrimaryChargeProgressChanged);
        ChargedShotgunActor->OnSecondaryChargeProgressChanged.RemoveDynamic(this, &US_ChargedShotgunViewModel::HandleSecondaryChargeProgressChanged);
    }
    ChargedShotgunActor.Reset();
    Super::Deinitialize();
}

void US_ChargedShotgunViewModel::HandlePrimaryChargeProgressChanged(float NewProgress)
{
    if (PrimaryChargeProgress != NewProgress)
    {
        PrimaryChargeProgress = NewProgress;
        OnWeaponViewModelUpdated.Broadcast();
    }
}

void US_ChargedShotgunViewModel::HandleSecondaryChargeProgressChanged(float NewProgress)
{
    if (SecondaryChargeProgress != NewProgress)
    {
        SecondaryChargeProgress = NewProgress;
        OnWeaponViewModelUpdated.Broadcast();
    }
}