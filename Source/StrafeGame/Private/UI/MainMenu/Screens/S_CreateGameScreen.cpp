// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/MainMenu/Screens/S_CreateGameScreen.h" // Updated include
#include "CommonButtonBase.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "Components/SpinBox.h"
#include "UI/MainMenu/MenuManagerSubsystem.h" // Updated include
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h" 

US_CreateGameScreen::US_CreateGameScreen()
{
}

void US_CreateGameScreen::NativeConstruct()
{
    Super::NativeConstruct();

    if (Btn_StartGame)
    {
        Btn_StartGame->OnClicked().AddUObject(this, &US_CreateGameScreen::OnStartGameClicked);
    }
    if (Btn_Back)
    {
        Btn_Back->OnClicked().AddUObject(this, &US_CreateGameScreen::OnBackClicked);
    }

    PopulateGameModes();

    if (CBox_GameMode && CBox_GameMode->GetOptionCount() > 0)
    {
        PopulateMapsForGameMode(CBox_GameMode->GetSelectedOption());
        CBox_GameMode->OnSelectionChanged.AddDynamic(this, &US_CreateGameScreen::PopulateMapsForGameMode);
    }
}

void US_CreateGameScreen::SetMenuManager_Implementation(UMenuManagerSubsystem* InMenuManager)
{
    MenuManager = InMenuManager;
}

void US_CreateGameScreen::PopulateGameModes()
{
    if (!CBox_GameMode) return;
    CBox_GameMode->ClearOptions();
    CBox_GameMode->AddOption(TEXT("Arena"));
    CBox_GameMode->AddOption(TEXT("Strafe"));
    CBox_GameMode->SetSelectedIndex(0);
}

void US_CreateGameScreen::PopulateMapsForGameMode(const FString& GameMode)
{
    if (!CBox_Map) return;
    CBox_Map->ClearOptions();

    // In a real project, this data would likely come from a DataTable or config file.
    // The map names should be the actual file names without the .umap extension.
    // The GameMode string is used to determine which maps to show.
    if (GameMode == TEXT("Arena"))
    {
        // Example: Assuming your maps are in /Content/Maps/Arena/
        CBox_Map->AddOption(TEXT("DM-Arena1")); // This would correspond to /Content/Maps/DM-Arena1.umap
        CBox_Map->AddOption(TEXT("DM-AnotherMap"));
    }
    else if (GameMode == TEXT("Strafe"))
    {
        // Example: Assuming your maps are in /Content/Maps/Strafe/
        CBox_Map->AddOption(TEXT("STR-MapA"));
        CBox_Map->AddOption(TEXT("STR-Speedway"));
    }

    if (CBox_Map->GetOptionCount() > 0)
    {
        CBox_Map->SetSelectedIndex(0);
    }
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
    if (!ValidateSettings())
    {
        return;
    }
    CreateGameSession();
}

void US_CreateGameScreen::OnBackClicked()
{
    if (MenuManager)
    {
        MenuManager->CloseTopmostScreen();
    }
}

void US_CreateGameScreen::CreateGameSession()
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            FOnlineSessionSettings SessionSettings;
            SessionSettings.NumPublicConnections = Spin_MaxPlayers ? static_cast<int>(Spin_MaxPlayers->GetValue()) : 8;
            SessionSettings.bShouldAdvertise = Chk_IsPublic ? Chk_IsPublic->IsChecked() : true;
            SessionSettings.bAllowJoinInProgress = true;
            SessionSettings.bIsLANMatch = false;
            SessionSettings.bUsesPresence = true;
            SessionSettings.bAllowInvites = true;

            // Use descriptive keys for your session settings
            SessionSettings.Set(FName(TEXT("GAMEMODE_NAME")), CBox_GameMode->GetSelectedOption(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
            SessionSettings.Set(SETTING_MAPNAME, CBox_Map->GetSelectedOption(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
            SessionSettings.Set(FName(TEXT("SESSION_DISPLAY_NAME")), Txt_GameName->GetText().ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

            FString Password = Txt_Password->GetText().ToString();
            if (!Password.IsEmpty())
            {
                SessionSettings.Set(FName(TEXT("SESSION_PASSWORD")), Password, EOnlineDataAdvertisementType::ViaOnlineService);
                SessionSettings.bIsPasswordProtected = true;
            }

            CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &US_CreateGameScreen::OnCreateSessionComplete);
            CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

            APlayerController* PC = GetOwningPlayer();
            if (PC && PC->GetLocalPlayer())
            {
                if (!SessionInterface->CreateSession(*PC->GetLocalPlayer()->GetPreferredUniqueNetId(), NAME_GameSession, SessionSettings))
                {
                    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
                    UE_LOG(LogTemp, Error, TEXT("Failed to start CreateSession."));
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("CreateSession call succeeded. Waiting for callback."));
                }
            }
        }
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

        // Construct the actual GameMode path. This needs to match your project structure.
        // Example: /Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode -> For a BP based GameMode
        // Or for C++: /Script/YourProject.YourGameModeClassName
        // For your structure, it might be something like:
        // /Script/StrafeGame.S_ArenaGameMode or /Script/StrafeGame.S_StrafeGameMode
        // If you are using Blueprint GameModes derived from these, the path would be like:
        // /Game/GameModes/Arena/BP_S_ArenaGameMode.BP_S_ArenaGameMode_C
        // Let's assume your C++ classes S_ArenaGameMode and S_StrafeGameMode are the ones to use.

        FString GameModePath;
        if (SelectedGameModeName == TEXT("Arena"))
        {
            GameModePath = TEXT("/Script/StrafeGame.S_ArenaGameMode");
        }
        else if (SelectedGameModeName == TEXT("Strafe"))
        {
            GameModePath = TEXT("/Script/StrafeGame.S_StrafeGameMode");
        }
        // Add more game modes if necessary

        if (GameModePath.IsEmpty())
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid GameMode selected for travel: %s"), *SelectedGameModeName);
            return;
        }

        // The map name for OpenLevel is just the map asset name, without /Game/Maps prefix usually.
        // However, UGameplayStatics::OpenLevel expects the short map name if it's in a standard map directory.
        // If your maps are in, e.g., /Content/Maps/, then just "DM-Arena1" is fine.
        FString URL = FString::Printf(TEXT("%s?listen?Game=%s"), *SelectedMapName, *GameModePath);

        UGameplayStatics::OpenLevel(GetWorld(), FName(*URL), true); // true for Absolute Travel

        if (MenuManager) MenuManager->ToggleMainMenu();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create session."));
    }
}