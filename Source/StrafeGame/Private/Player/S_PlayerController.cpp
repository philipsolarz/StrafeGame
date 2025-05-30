// Copyright Epic Games, Inc. All Rights Reserved.
#include "Player/S_PlayerController.h"
#include "UI/S_PlayerHUDManager.h" // Include the new HUD Manager
#include "Engine/World.h"       // For GetWorld()
#include "Net/UnrealNetwork.h"  // For Role checks

AS_PlayerController::AS_PlayerController()
{
    // Constructor
    PlayerHUDManagerInstance = nullptr;
}

void AS_PlayerController::BeginPlay()
{
    Super::BeginPlay();

    // The HUD should only be created for locally controlled player controllers.
    if (IsLocalController())
    {
        UE_LOG(LogTemp, Log, TEXT("AS_PlayerController %s (Local): BeginPlay - Attempting to create HUD Manager."), *GetNameSafe(this));
        CreatePlayerHUDManager();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AS_PlayerController %s (Remote): BeginPlay - Not creating HUD Manager."), *GetNameSafe(this));
    }
}

void AS_PlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    // OnPossess is server-authoritative for game logic.
    // HUD creation is client-side and handled in BeginPlay or via a specific client function.
    UE_LOG(LogTemp, Log, TEXT("AS_PlayerController %s: Possessed Pawn %s (Server)."), *GetNameSafe(this), *GetNameSafe(InPawn));
}

void AS_PlayerController::OnUnPossess()
{
    Super::OnUnPossess();
    UE_LOG(LogTemp, Log, TEXT("AS_PlayerController %s: Unpossessed Pawn (Server)."), *GetNameSafe(this));
}

void AS_PlayerController::CreatePlayerHUDManager()
{
    // Ensure this runs only on the client that owns this controller.
    if (!IsLocalController())
    {
        return;
    }

    // Destroy previous HUD manager instance if it somehow exists (e.g., during seamless travel if not handled properly)
    if (PlayerHUDManagerInstance)
    {
        UE_LOG(LogTemp, Log, TEXT("AS_PlayerController %s: Destroying existing PlayerHUDManagerInstance."), *GetNameSafe(this));
        PlayerHUDManagerInstance->Destroy();
        PlayerHUDManagerInstance = nullptr;
    }

    if (PlayerHUDManagerClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this; // The PlayerController owns the HUD Manager
        SpawnParams.Instigator = GetInstigator(); // Usually the Pawn controlled by this PC
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        PlayerHUDManagerInstance = GetWorld()->SpawnActor<AS_PlayerHUDManager>(PlayerHUDManagerClass, SpawnParams);

        if (PlayerHUDManagerInstance)
        {
            PlayerHUDManagerInstance->Initialize(this); // Pass this PlayerController to the manager
            UE_LOG(LogTemp, Log, TEXT("AS_PlayerController %s: Spawned and Initialized PlayerHUDManagerInstance: %s of class %s"),
                *GetNameSafe(this), *GetNameSafe(PlayerHUDManagerInstance), *GetNameSafe(PlayerHUDManagerClass));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AS_PlayerController %s: Failed to spawn PlayerHUDManager from class %s."), *GetNameSafe(this), *GetNameSafe(PlayerHUDManagerClass));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_PlayerController %s: PlayerHUDManagerClass is not set in defaults. Cannot create HUD Manager."), *GetNameSafe(this));
    }
}