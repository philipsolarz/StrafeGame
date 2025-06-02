#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/S_GameModeHUDViewModelBase.h"
#include "UI/ViewModels/S_KillfeedItemViewModel.h" // Include to get FKillfeedEventData and US_KillfeedItemViewModel
#include "S_ArenaHUDViewModel.generated.h"

class AS_ArenaGameState;
class AS_ArenaPlayerState;
// class UTexture2D; // No longer needed here if FKillfeedEventData handles its TSoftObjectPtr

// struct FKillfeedMessageViewModel -> This definition should be REMOVED from here if it exists.
// It's now FKillfeedEventData in S_KillfeedItemViewModel.h


UCLASS()
class STRAFEGAME_API US_ArenaHUDViewModel : public US_GameModeHUDViewModelBase
{
    GENERATED_BODY()

public:
    virtual void Initialize(AS_PlayerController* InOwningPlayerController) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintReadOnly, Category = "ArenaViewModel")
    int32 FragLimit;

    UPROPERTY(BlueprintReadOnly, Category = "ArenaViewModel")
    int32 PlayerFrags;

    UPROPERTY(BlueprintReadOnly, Category = "ArenaViewModel")
    int32 PlayerDeaths;

    UPROPERTY(BlueprintReadOnly, Category = "ArenaViewModel")
    FString LeaderName;

    UPROPERTY(BlueprintReadOnly, Category = "ArenaViewModel")
    int32 LeaderFrags;

    UPROPERTY(BlueprintReadOnly, Category = "ArenaViewModel")
    TArray<TObjectPtr<US_KillfeedItemViewModel>> KillfeedItemViewModels; // This now uses the UObject ViewModel


protected:
    TWeakObjectPtr<AS_ArenaGameState> ArenaGameState;
    TWeakObjectPtr<AS_ArenaPlayerState> LocalArenaPlayerState;

    FDelegateHandle KillfeedGameStateUpdatedHandle;

    virtual void RefreshGameModeData() override;
    void UpdateArenaSpecificData();
    void UpdateKillfeed();

    UFUNCTION()
    void HandleLocalPlayerScoreUpdated(AS_ArenaPlayerState* InPlayerState, int32 NewFrags, int32 NewDeaths);

    UFUNCTION()
    void HandleGameStateKillfeedUpdated();
};