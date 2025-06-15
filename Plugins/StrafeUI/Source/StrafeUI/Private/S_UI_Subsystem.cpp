#include "S_UI_Subsystem.h"
#include "S_UI_Settings.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "Components/NamedSlot.h"

#include "S_UI_AssetManager.h"
#include "S_UI_Navigator.h"
#include "S_UI_InputController.h"
#include "S_UI_ModalStack.h"
#include "S_UI_PlayerController.h"
#include "S_UI_OnlineSessionManager.h"
#include "UI/S_UI_RootWidget.h"
#include "UI/S_UI_MainMenuWidget.h"

void US_UI_Subsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Create the manager instances.
    AssetManager = NewObject<US_UI_AssetManager>(this);
    Navigator = NewObject<US_UI_Navigator>(this);
    SessionManager = NewObject<US_UI_OnlineSessionManager>(this);

    // Initialize the session manager
    SessionManager->Initialize();

    UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem Initialized"));
}

void US_UI_Subsystem::Deinitialize()
{
    // Clean up the session manager first
    if (SessionManager)
    {
        SessionManager->Shutdown();
        SessionManager = nullptr;
    }

    if (UIRootWidget)
    {
        UIRootWidget->RemoveFromParent();
        UIRootWidget = nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem Deinitialized"));
    Super::Deinitialize();
}

void US_UI_Subsystem::InitializeUIForPlayer(AS_UI_PlayerController* PlayerController)
{
    if (!PlayerController || UIRootWidget)
    {
        return;
    }

    InitializingPlayer = PlayerController;
    const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();

    if (!Settings)
    {
        UE_LOG(LogTemp, Error, TEXT("S_UI_Subsystem: Cannot find StrafeUISettings!"));
        return;
    }

    // Initialize asset manager and bind completion callback.
    AssetManager->Initialize(Settings);
    AssetManager->OnAssetsLoaded.BindUObject(this, &US_UI_Subsystem::FinalizeUIInitialization);
    AssetManager->StartAssetsLoading();
}

void US_UI_Subsystem::FinalizeUIInitialization()
{
    UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem: Finalizing UI setup..."));

    if (!InitializingPlayer.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("S_UI_Subsystem: PlayerController became invalid during asset loading. Aborting UI setup."));
        return;
    }

    AS_UI_PlayerController* PlayerController = InitializingPlayer.Get();
    const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();

    // --- Initialize Modal Stack ---
    if (!ModalStack)
    {
        if (TSubclassOf<US_UI_ModalStack> LoadedModalStackClass = TSubclassOf<US_UI_ModalStack>(Settings->ModalStackClass.Get()))
        {
            ModalStack = NewObject<US_UI_ModalStack>(this, LoadedModalStackClass);
            ModalStack->Initialize(this, Settings->ModalWidgetClass);
        }
    }

    // --- Initialize Input Controller ---
    if (TSubclassOf<US_UI_InputController> LoadedInputControllerClass = TSubclassOf<US_UI_InputController>(Settings->InputControllerClass.Get()))
    {
        InputController = NewObject<US_UI_InputController>(this, LoadedInputControllerClass);
        if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
        {
            InputController->Initialize(this, EIC, Settings);
        }
    }

    // --- Create Root Widget ---
    if (const TSubclassOf<US_UI_RootWidget> RootClass = TSubclassOf<US_UI_RootWidget>(Settings->RootWidgetClass.Get()))
    {
        UIRootWidget = CreateWidget<US_UI_RootWidget>(PlayerController, RootClass);
        if (UIRootWidget)
        {
            UIRootWidget->AddToViewport();

            // Create and add the persistent Main Menu widget
            if (const TSubclassOf<US_UI_MainMenuWidget> MainMenuClass = TSubclassOf<US_UI_MainMenuWidget>(Settings->MainMenuWidgetClass.Get()))
            {
                US_UI_MainMenuWidget* MainMenuWidget = CreateWidget<US_UI_MainMenuWidget>(PlayerController, MainMenuClass);
                if (MainMenuWidget && UIRootWidget->GetMainMenuSlot())
                {
                    UIRootWidget->GetMainMenuSlot()->AddChild(MainMenuWidget);
                }
            }
        }
    }

    // --- Initialize Navigator ---
    Navigator->Initialize(UIRootWidget, AssetManager);
}


void US_UI_Subsystem::RequestModal(const F_UIModalPayload& Payload, const FOnModalDismissedSignature& OnDismissedCallback)
{
    if (!AssetManager || !AssetManager->AreAssetsLoaded())
    {
        UE_LOG(LogTemp, Warning, TEXT("RequestModal called before core assets are loaded. The request will be ignored."));
        return;
    }

    if (ModalStack)
    {
        ModalStack->QueueModal(Payload, OnDismissedCallback);
        UE_LOG(LogTemp, Verbose, TEXT("Modal requested with message: %s"), *Payload.Message.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("RequestModal failed: ModalStack is not initialized!"));
    }
}