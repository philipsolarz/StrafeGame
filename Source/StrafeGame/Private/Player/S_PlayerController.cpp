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

AS_PlayerHUDManager* AS_PlayerController::GetPlayerHUDManagerInstance() const
{
    return PlayerHUDManagerInstance.Get(); // .Get() is used for TObjectPtr to get the raw pointer
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
    if (!IsLocalController())
    {
        return;
    }

    if (PlayerHUDManagerInstance)
    {
        UE_LOG(LogTemp, Log, TEXT("AS_PlayerController %s: Destroying existing PlayerHUDManagerInstance."), *GetNameSafe(this));
        PlayerHUDManagerInstance->Destroy();
        PlayerHUDManagerInstance = nullptr;
    }

    if (PlayerHUDManagerClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        PlayerHUDManagerInstance = GetWorld()->SpawnActor<AS_PlayerHUDManager>(PlayerHUDManagerClass, SpawnParams);

        if (PlayerHUDManagerInstance)
        {
            PlayerHUDManagerInstance->Initialize(this);
            UE_LOG(LogTemp, Log, TEXT("AS_PlayerController %s: Spawned and Initialized PlayerHUDManagerInstance: %s of class %s"),
                *GetNameSafe(this), *GetNameSafe(PlayerHUDManagerInstance.Get()), *GetNameSafe(PlayerHUDManagerClass)); // Use .Get() for logging TObjectPtr
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