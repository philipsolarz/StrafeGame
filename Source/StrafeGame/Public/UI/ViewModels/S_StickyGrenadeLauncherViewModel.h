// Source/StrafeGame/Public/UI/ViewModels/S_StickyGrenadeLauncherViewModel.h
#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/S_WeaponViewModel.h"
#include "S_StickyGrenadeLauncherViewModel.generated.h"

class AS_StickyGrenadeLauncher;
class AS_Weapon;
class AS_PlayerController;

UCLASS()
class STRAFEGAME_API US_StickyGrenadeLauncherViewModel : public US_WeaponViewModel
{
    GENERATED_BODY()

public:
    virtual void Initialize(AS_PlayerController* InOwningPlayerController, AS_Weapon* InWeaponActor) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintReadOnly, Category = "StickyLauncherViewModel")
    int32 ActiveStickyGrenadeCount;

protected:
    TWeakObjectPtr<AS_StickyGrenadeLauncher> StickyLauncherActor;

    UFUNCTION()
    void HandleActiveStickyCountChanged(int32 NewCount);
};