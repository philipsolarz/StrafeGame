// Source/StrafeGame/Private/UI/MainMenu/Screens/S_CreateGameScreen.cpp
#include "UI/MainMenu/Screens/S_CreateGameScreen.h"
#include "CommonButtonBase.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "Components/SpinBox.h"
#include "UI/MainMenu/S_MainMenuPlayerController.h" // Changed from MenuManagerSubsystem
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/GameInstance.h" 
#include "Engine/LocalPlayer.h"

#ifndef SETTING_MAPNAME
#define SETTING_MAPNAME FName(TEXT("MAPNAME"))
#endif

US_CreateGameScreen::US_CreateGameScreen() {}

void US_CreateGameScreen::NativeConstruct()
{
    Super::NativeConstruct();

    if (Btn_StartGame) Btn_StartGame->OnClicked().AddUObject(this, &US_CreateGameScreen::OnStartGameClicked);
    if (Btn_Back) Btn_Back->OnClicked().AddUObject(this, &US_CreateGameScreen::OnBackClicked);

    PopulateGameModes();

    if (CBox_GameMode && CBox_GameMode->GetOptionCount() > 0)
    {
        CBox_GameMode->OnSelectionChanged.AddDynamic(this, &US_CreateGameScreen::OnGameModeSelectionChanged);
        // Initial population based on default selection
        if (!CBox_GameMode->GetSelectedOption().IsEmpty())
        {
            PopulateMapsForGameMode(CBox_GameMode->GetSelectedOption());
        }
    }
}

// Updated interface implementation
void US_CreateGameScreen::SetMainMenuPlayerController_Implementation(AS_MainMenuPlayerController* InPlayerController)
{
    OwningMainMenuPlayerController = InPlayerController;
}

void US_CreateGameScreen::PopulateGameModes()
{
    if (!CBox_GameMode) return;
    CBox_GameMode->ClearOptions();
    CBox_GameMode->AddOption(TEXT("Arena"));
    CBox_GameMode->AddOption(TEXT("Strafe"));
    if (CBox_GameMode->GetOptionCount() > 0) CBox_GameMode->SetSelectedIndex(0);
}

void US_CreateGameScreen::OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    PopulateMapsForGameMode(SelectedItem);
}

void US_CreateGameScreen::PopulateMapsForGameMode(const FString& GameMode)
{
    if (!CBox_Map) return;
    CBox_Map->ClearOptions();
    if (GameMode == TEXT("Arena"))
    {
        CBox_Map->AddOption(TEXT("DM-Arena1"));
        CBox_Map->AddOption(TEXT("DM_AnotherMap"));
    }
    else if (GameMode == TEXT("Strafe"))
    {
        CBox_Map->AddOption(TEXT("STR-MapA"));
        CBox_Map->AddOption(TEXT("STR_Speedway"));
    }
    if (CBox_Map->GetOptionCount() > 0) CBox_Map->SetSelectedIndex(0);
}

bool US_CreateGameScreen::ValidateSettings() const
{
    if (Txt_GameName && Txt_GameName->GetText().IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Create Game Validation: Game Name is empty."));
        return false;
    }
    if (CBox_Map && CBox_Map->GetSelectedOption().IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Create Game Validation: No map selected."));
        return false;
    }
    return true;
}

void US_CreateGameScreen::OnStartGameClicked()
{
    if (!ValidateSettings()) return;
    UE_LOG(LogTemp, Log, TEXT("Start Game Clicked. Settings Validated. Creating session..."));
    CreateGameSession();
}

void US_CreateGameScreen::OnBackClicked()
{
    if (OwningMainMenuPlayerController.IsValid())
    {
        OwningMainMenuPlayerController->CloseTopmostScreen();
    }
}

void US_CreateGameScreen::CreateGameSession()
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (!OnlineSub)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateGameSession: OnlineSubsystem is not available."));
        return;
    }

    IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("CreateGameSession: OnlineSessionInterface is not valid."));
        return;
    }

    FOnlineSessionSettings SessionSettings;
    SessionSettings.NumPublicConnections = Spin_MaxPlayers ? static_cast<int32>(Spin_MaxPlayers->GetValue()) : 8;
    SessionSettings.bShouldAdvertise = Chk_IsPublic ? Chk_IsPublic->IsChecked() : true;
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bIsLANMatch = false;
    SessionSettings.bUsesPresence = true;
    SessionSettings.bAllowInvites = true;
    SessionSettings.BuildUniqueId = 0; // Replace with a proper build ID

    SessionSettings.Set(FName(TEXT("GAMEMODE_NAME")), CBox_GameMode->GetSelectedOption(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    SessionSettings.Set(SETTING_MAPNAME, CBox_Map->GetSelectedOption(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    SessionSettings.Set(FName(TEXT("SESSION_DISPLAY_NAME")), Txt_GameName->GetText().ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    FString Password = Txt_Password->GetText().ToString();
    SessionSettings.Set(FName(TEXT("SESSION_PASSWORD_PROTECTED")), !Password.IsEmpty(), EOnlineDataAdvertisementType::ViaOnlineService);

    CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &US_CreateGameScreen::OnCreateSessionComplete);
    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

    APlayerController* PC = GetOwningPlayer();
    if (!PC || !PC->GetLocalPlayer())
    {
        UE_LOG(LogTemp, Error, TEXT("CreateGameSession: OwningPlayerController or LocalPlayer is null."));
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        return;
    }

    const FUniqueNetIdRepl LocalPlayerNetId = PC->GetLocalPlayer()->GetPreferredUniqueNetId();
    if (!LocalPlayerNetId.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("CreateGameSession: LocalPlayerNetId is not valid."));
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        return;
    }

    if (!SessionInterface->CreateSession(*LocalPlayerNetId, NAME_GameSession, SessionSettings))
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        UE_LOG(LogTemp, Error, TEXT("Failed to initiate CreateSession call."));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("CreateSession call initiated successfully. Waiting for callback."));
    }
}

void US_CreateGameScreen::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        }
    }

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Session creation successful: %s"), *SessionName.ToString());
        FString SelectedMapName = CBox_Map->GetSelectedOption();
        FString SelectedGameModeName = CBox_GameMode->GetSelectedOption();
        FString GameModePathOption;

        if (SelectedGameModeName == TEXT("Arena")) GameModePathOption = TEXT("?Game=/Game/GameModes/Arena/BP_S_ArenaGameMode.BP_S_ArenaGameMode_C");
        else if (SelectedGameModeName == TEXT("Strafe")) GameModePathOption = TEXT("?Game=/Game/GameModes/Strafe/BP_S_StrafeGameMode.BP_S_StrafeGameMode_C");

        if (GameModePathOption.IsEmpty())
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid GameMode selected or path not configured for travel: %s"), *SelectedGameModeName);
            return;
        }

        FString URL = FString::Printf(TEXT("/Game/Maps/%s%s?listen"), *SelectedMapName, *GameModePathOption);
        UE_LOG(LogTemp, Log, TEXT("Attempting to open level with URL: %s"), *URL);
        UGameplayStatics::OpenLevel(GetWorld(), FName(*URL), true);

        if (OwningMainMenuPlayerController.IsValid()) OwningMainMenuPlayerController->CloseFullMenu(); // Close entire menu
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create session. SessionName: %s"), *SessionName.ToString());
    }
}