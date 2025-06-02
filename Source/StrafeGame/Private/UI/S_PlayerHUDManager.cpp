// Source/StrafeGame/Private/UI/S_PlayerHUDManager.cpp
#include "UI/S_PlayerHUDManager.h"
#include "Blueprint/UserWidget.h"
#include "Player/S_PlayerController.h"
#include "Engine/World.h"
#include "GameModes/S_GameStateBase.h" 
#include "GameModes/Arena/S_ArenaGameState.h"
#include "GameModes/Strafe/S_StrafeGameState.h"

// Include ViewModel headers
#include "UI/ViewModels/S_PlayerHUDViewModel.h"
#include "UI/ViewModels/S_GameModeHUDViewModelBase.h"
#include "UI/ViewModels/S_ArenaHUDViewModel.h"
#include "UI/ViewModels/S_StrafeHUDViewModel.h"
#include "UI/ViewModels/S_AnnouncerViewModel.h"
#include "UI/ViewModels/S_ScreenEffectsViewModel.h"


AS_PlayerHUDManager::AS_PlayerHUDManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;

    ArenaHUDViewModelClass = US_ArenaHUDViewModel::StaticClass();
    StrafeHUDViewModelClass = US_StrafeHUDViewModel::StaticClass();
}

void AS_PlayerHUDManager::BeginPlay()
{
    Super::BeginPlay();
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

    CreateViewModels();
    InitializeViewModels();

    if (MainHUDWidgetClass)
    {
        if (!MainHUDWidget)
        {
            MainHUDWidget = CreateWidget<UUserWidget>(OwningStrafePC.Get(), MainHUDWidgetClass);
            UE_LOG(LogTemp, Log, TEXT("AS_PlayerHUDManager %s: Created MainHUDWidget of class %s."), *GetNameSafe(this), *GetNameSafe(MainHUDWidgetClass));
        }

        if (MainHUDWidget && !MainHUDWidget->IsInViewport())
        {
            MainHUDWidget->AddToViewport();
            UE_LOG(LogTemp, Log, TEXT("AS_PlayerHUDManager %s: MainHUDWidget (%s) added to viewport."), *GetNameSafe(this), *GetNameSafe(MainHUDWidget));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_PlayerHUDManager %s: MainHUDWidgetClass is not set! Cannot create the main HUD."), *GetNameSafe(this));
    }

    OnMatchStateChanged();
}

void AS_PlayerHUDManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DeinitializeViewModels();

    if (MainHUDWidget)
    {
        MainHUDWidget->RemoveFromParent();
        MainHUDWidget = nullptr;
        UE_LOG(LogTemp, Log, TEXT("AS_PlayerHUDManager %s: MainHUDWidget removed from parent and cleared."), *GetNameSafe(this));
    }
    Super::EndPlay(EndPlayReason);
}

void AS_PlayerHUDManager::CreateViewModels()
{
    if (!OwningStrafePC) return; // Corrected check

    PlayerHUDViewModel = NewObject<US_PlayerHUDViewModel>(this, TEXT("PlayerHUDViewModel"));
    AnnouncerViewModel = NewObject<US_AnnouncerViewModel>(this, TEXT("AnnouncerViewModel"));
    ScreenEffectsViewModel = NewObject<US_ScreenEffectsViewModel>(this, TEXT("ScreenEffectsViewModel"));
}

void AS_PlayerHUDManager::InitializeViewModels()
{
    if (!OwningStrafePC) return; // Corrected check

    if (PlayerHUDViewModel) PlayerHUDViewModel->Initialize(OwningStrafePC.Get());
    if (AnnouncerViewModel) AnnouncerViewModel->Initialize(OwningStrafePC.Get());
    if (ScreenEffectsViewModel) ScreenEffectsViewModel->Initialize(OwningStrafePC.Get());
}

void AS_PlayerHUDManager::DeinitializeViewModels()
{
    if (PlayerHUDViewModel) PlayerHUDViewModel->Deinitialize();
    if (GameModeHUDViewModel) GameModeHUDViewModel->Deinitialize();
    if (AnnouncerViewModel) AnnouncerViewModel->Deinitialize();
    if (ScreenEffectsViewModel) ScreenEffectsViewModel->Deinitialize();
}


AS_PlayerController* AS_PlayerHUDManager::GetOwningStrafePlayerController() const
{
    return OwningStrafePC.Get();
}

void AS_PlayerHUDManager::ShowGameAnnouncement(const FText& Message, float Duration)
{
    if (AnnouncerViewModel)
    {
        AnnouncerViewModel->ShowAnnouncement(Message, Duration);
    }
}

void AS_PlayerHUDManager::OnMatchStateChanged()
{
    if (!OwningStrafePC || !GetWorld()) return; // Corrected check

    AS_GameStateBase* CurrentGameState = GetWorld()->GetGameState<AS_GameStateBase>();
    TSubclassOf<US_GameModeHUDViewModelBase> TargetViewModelClass = nullptr;

    if (Cast<AS_ArenaGameState>(CurrentGameState))
    {
        TargetViewModelClass = ArenaHUDViewModelClass;
    }
    else if (Cast<AS_StrafeGameState>(CurrentGameState))
    {
        TargetViewModelClass = StrafeHUDViewModelClass;
    }

    if (GameModeHUDViewModel && GameModeHUDViewModel->GetClass() == TargetViewModelClass)
    {
        GameModeHUDViewModel->Initialize(OwningStrafePC.Get());
        return;
    }

    if (GameModeHUDViewModel)
    {
        GameModeHUDViewModel->Deinitialize();
        GameModeHUDViewModel = nullptr;
    }

    if (TargetViewModelClass)
    {
        GameModeHUDViewModel = NewObject<US_GameModeHUDViewModelBase>(this, TargetViewModelClass);
        if (GameModeHUDViewModel)
        {
            GameModeHUDViewModel->Initialize(OwningStrafePC.Get());
            UE_LOG(LogTemp, Log, TEXT("AS_PlayerHUDManager: Switched to GameModeViewModel: %s"), *GameModeHUDViewModel->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AS_PlayerHUDManager: No specific ViewModel class found for current GameState type."));
    }
}