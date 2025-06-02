// Source/StrafeGame/Private/UI/ViewModels/S_StickyGrenadeLauncherViewModel.cpp
#include "UI/ViewModels/S_StickyGrenadeLauncherViewModel.h"
#include "Weapons/StickyGrenadeLauncher/S_StickyGrenadeLauncher.h"
#include "Weapons/S_Projectile.h" 
#include "Player/S_PlayerController.h"
// Removed TimerManager and Engine/World

void US_StickyGrenadeLauncherViewModel::Initialize(AS_PlayerController* InOwningPlayerController, AS_Weapon* InWeaponActor)
{
    Super::Initialize(InOwningPlayerController, InWeaponActor); // Calls base US_WeaponViewModel::Initialize
    StickyLauncherActor = Cast<AS_StickyGrenadeLauncher>(WeaponActor.Get()); // Get from base
    ActiveStickyGrenadeCount = 0;

    if (StickyLauncherActor.IsValid())
    {
        StickyLauncherActor->OnActiveProjectilesChanged.AddDynamic(this, &US_StickyGrenadeLauncherViewModel::HandleActiveStickyCountChanged);
        // Initial update
        ActiveStickyGrenadeCount = StickyLauncherActor->GetValidActiveProjectileCount();
        OnWeaponViewModelUpdated.Broadcast(); // From base class
    }
}

void US_StickyGrenadeLauncherViewModel::Deinitialize()
{
    if (StickyLauncherActor.IsValid())
    {
        StickyLauncherActor->OnActiveProjectilesChanged.RemoveDynamic(this, &US_StickyGrenadeLauncherViewModel::HandleActiveStickyCountChanged);
    }
    StickyLauncherActor.Reset();
    Super::Deinitialize();
}

void US_StickyGrenadeLauncherViewModel::HandleActiveStickyCountChanged(int32 NewCount)
{
    if (ActiveStickyGrenadeCount != NewCount)
    {
        ActiveStickyGrenadeCount = NewCount;
        OnWeaponViewModelUpdated.Broadcast();
    }
}