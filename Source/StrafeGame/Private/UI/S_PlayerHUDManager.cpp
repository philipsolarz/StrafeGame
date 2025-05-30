// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/S_PlayerHUDManager.h"
#include "Blueprint/UserWidget.h"
#include "Player/S_PlayerController.h" // Include the actual header
#include "Engine/World.h" // For GetWorld()

AS_PlayerHUDManager::AS_PlayerHUDManager()
{
    PrimaryActorTick.bCanEverTick = false; // No tick needed for now
    bReplicates = false; // This is a client-side actor, owned by the PlayerController
}

void AS_PlayerHUDManager::BeginPlay()
{
    Super::BeginPlay();
    // Initialization logic is called explicitly via Initialize by the PlayerController
    // to ensure OwningStrafePC is set.
}

void AS_PlayerHUDManager::Initialize(AS_PlayerController* InOwningPlayerController)
{
    if (!InOwningPlayerController || !InOwningPlayerController->IsLocalController())
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_PlayerHUDManager::Initialize: Attempted to initialize with an invalid or non-local OwningPlayerController for %s."), *GetNameSafe(this));
        return;
    }

    OwningStrafePC = InOwningPlayerController;
    UE_LOG(LogTemp, Log, TEXT("AS_PlayerHUDManager %s initialized with OwningPlayerController: %s"), *GetNameSafe(this), *GetNameSafe(OwningStrafePC));

    if (MainHUDWidgetClass)
    {
        if (!MainHUDWidget) // Check if it hasn't been created yet
        {
            MainHUDWidget = CreateWidget<UUserWidget>(OwningStrafePC, MainHUDWidgetClass);
            UE_LOG(LogTemp, Log, TEXT("AS_PlayerHUDManager %s: Created MainHUDWidget of class %s."), *GetNameSafe(this), *GetNameSafe(MainHUDWidgetClass));
        }

        if (MainHUDWidget && !MainHUDWidget->IsInViewport())
        {
            MainHUDWidget->AddToViewport();
            UE_LOG(LogTemp, Log, TEXT("AS_PlayerHUDManager %s: MainHUDWidget (%s) added to viewport."), *GetNameSafe(this), *GetNameSafe(MainHUDWidget));
        }
        else if (MainHUDWidget && MainHUDWidget->IsInViewport())
        {
            UE_LOG(LogTemp, Log, TEXT("AS_PlayerHUDManager %s: MainHUDWidget (%s) was already in viewport."), *GetNameSafe(this), *GetNameSafe(MainHUDWidget));
        }
        else if (!MainHUDWidget)
        {
            UE_LOG(LogTemp, Error, TEXT("AS_PlayerHUDManager %s: Failed to create MainHUDWidget from class %s."), *GetNameSafe(this), *GetNameSafe(MainHUDWidgetClass));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_PlayerHUDManager %s: MainHUDWidgetClass is not set! Cannot create the main HUD."), *GetNameSafe(this));
    }
}

void AS_PlayerHUDManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (MainHUDWidget)
    {
        MainHUDWidget->RemoveFromParent();
        MainHUDWidget = nullptr;
        UE_LOG(LogTemp, Log, TEXT("AS_PlayerHUDManager %s: MainHUDWidget removed from parent and cleared."), *GetNameSafe(this));
    }
    Super::EndPlay(EndPlayReason);
}

AS_PlayerController* AS_PlayerHUDManager::GetOwningStrafePlayerController() const
{
    return OwningStrafePC;
}