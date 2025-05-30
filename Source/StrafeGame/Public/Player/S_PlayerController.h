// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "S_PlayerController.generated.h"

class AS_PlayerHUDManager; // Forward declaration

UCLASS()
class STRAFEGAME_API AS_PlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AS_PlayerController();

protected:
    /** Called when the game starts or when the player controller is spawned/initialized.
     * For client controllers, this (or ReceivedPlayer) is a good place to set up client-specific things like the HUD.
     */
    virtual void BeginPlay() override;

    /** Called after this PlayerController has possessed a Pawn. (Server-Only for game logic) */
    virtual void OnPossess(APawn* InPawn) override;

    /** Called when this PlayerController unpossesses its Pawn. (Server-Only for game logic) */
    virtual void OnUnPossess() override;


    /** Class of the PlayerHUDManager to spawn. Should be set in Blueprint defaults of a derived PlayerController. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD", Meta = (DisplayName = "Player HUD Manager Class"))
    TSubclassOf<AS_PlayerHUDManager> PlayerHUDManagerClass;

    /** Instance of the PlayerHUDManager. Created on the client. */
    UPROPERTY(BlueprintReadOnly, Category = "HUD", Transient) // Transient as it's created at runtime and client-side
        TObjectPtr<AS_PlayerHUDManager> PlayerHUDManagerInstance;

    /**
     * Creates and initializes the HUD manager. Called on the client.
     * This will in turn create the actual UMG HUD.
     */
    virtual void CreatePlayerHUDManager();
};