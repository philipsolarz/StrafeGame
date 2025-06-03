// Source/StrafeGame/Private/UI/MainMenu/Screens/S_CreateGameScreen.cpp
#include "UI/MainMenu/Screens/S_CreateGameScreen.h"
#include "CommonButtonBase.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "Components/SpinBox.h"
#include "UI/MainMenu/MenuManagerSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h" // For FOnlineSessionSettings and SETTING_MAPNAME
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h" // Included for completeness, though not directly used

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
        // Bind to the new intermediate function
        CBox_GameMode->OnSelectionChanged.AddDynamic(this, &US_CreateGameScreen::OnGameModeSelectionChanged);
        // Initial population based on default selection
        PopulateMapsForGameMode(CBox_GameMode->GetSelectedOption());
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
    CBox_GameMode->AddOption(TEXT("Arena")); // These should match the strings used in OnCreateSessionComplete
    CBox_GameMode->AddOption(TEXT("Strafe"));
    if (CBox_GameMode->GetOptionCount() > 0)
    {
        CBox_GameMode->SetSelectedIndex(0); // Default to first option
    }
}

// New intermediate function with the correct signature for the delegate
void US_CreateGameScreen::OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    // Call the original logic function
    PopulateMapsForGameMode(SelectedItem);
}

void US_CreateGameScreen::PopulateMapsForGameMode(const FString& GameMode)
{
    if (!CBox_Map) return;
    CBox_Map->ClearOptions();

    // Map names should be exactly as they appear in the content browser (without .umap extension)
    if (GameMode == TEXT("Arena"))
    {
        CBox_Map->AddOption(TEXT("DM-Arena1"));
        CBox_Map->AddOption(TEXT("DM_AnotherMap")); // Example: Use correct asset names
    }
    else if (GameMode == TEXT("Strafe"))
    {
        CBox_Map->AddOption(TEXT("STR-MapA"));
        CBox_Map->AddOption(TEXT("STR_Speedway")); // Example: Use correct asset names
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
        // Optionally, provide UI feedback here
        return false;
    }
    if (CBox_Map && CBox_Map->GetSelectedOption().IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Create Game Validation: No map selected."));
        // Optionally, provide UI feedback here
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
    UE_LOG(LogTemp, Log, TEXT("Start Game Clicked. Settings Validated. Creating session..."));
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
            SessionSettings.NumPublicConnections = Spin_MaxPlayers ? static_cast<int32>(Spin_MaxPlayers->GetValue()) : 8;
            SessionSettings.bShouldAdvertise = Chk_IsPublic ? Chk_IsPublic->IsChecked() : true;
            SessionSettings.bAllowJoinInProgress = true;
            SessionSettings.bIsLANMatch = false; // Change to true for LAN games
            SessionSettings.bUsesPresence = true; // Recommended for advertising through player presence
            SessionSettings.bAllowInvites = true;
            SessionSettings.BuildUniqueId = Get टाइप(); // Helps with version matching

            // Set custom session properties
            SessionSettings.Set(FName(TEXT("GAMEMODE_NAME")), CBox_GameMode->GetSelectedOption(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
            SessionSettings.Set(SETTING_MAPNAME, CBox_Map->GetSelectedOption(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
            SessionSettings.Set(FName(TEXT("SESSION_DISPLAY_NAME")), Txt_GameName->GetText().ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

            FString Password = Txt_Password->GetText().ToString();
            if (!Password.IsEmpty())
            {
                SessionSettings.Set(FName(TEXT("SESSION_PASSWORD_PROTECTED")), true, EOnlineDataAdvertisementType::ViaOnlineService);
                // The actual password is not typically stored in session settings directly for security.
                // Password check should be handled by game logic on join attempt.
            }
            else
            {
                SessionSettings.Set(FName(TEXT("SESSION_PASSWORD_PROTECTED")), false, EOnlineDataAdvertisementType::ViaOnlineService);
            }


            CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &US_CreateGameScreen::OnCreateSessionComplete);
            CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

            APlayerController* PC = GetOwningPlayer();
            if (PC && PC->GetLocalPlayer())
            {
                if (!SessionInterface->CreateSession(*PC->GetLocalPlayer()->GetPreferredUniqueNetId(), NAME_GameSession, SessionSettings))
                {
                    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
                    UE_LOG(LogTemp, Error, TEXT("Failed to initiate CreateSession call."));
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("CreateSession call initiated successfully. Waiting for callback."));
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("CreateGameSession: OwningPlayerController or LocalPlayer is null."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("CreateGameSession: OnlineSessionInterface is not valid."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CreateGameSession: OnlineSubsystem is not available."));
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
        FString GameModePathOption; // This will be like "?Game=/Game/Blueprints/MyGameMode.MyGameMode_C"

        // These paths must match your Blueprint GameMode assets if you're using BPs,
        // or /Script/YourModule.YourC++GameMode if using C++ directly.
        if (SelectedGameModeName == TEXT("Arena"))
        {
            // Assuming your BP GameMode for Arena is at this path
            GameModePathOption = TEXT("?Game=/Game/GameModes/Arena/BP_S_ArenaGameMode.BP_S_ArenaGameMode_C");
        }
        else if (SelectedGameModeName == TEXT("Strafe"))
        {
            // Assuming your BP GameMode for Strafe is at this path
            GameModePathOption = TEXT("?Game=/Game/GameModes/Strafe/BP_S_StrafeGameMode.BP_S_StrafeGameMode_C");
        }

        if (GameModePathOption.IsEmpty())
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid GameMode selected or path not configured for travel: %s"), *SelectedGameModeName);
            return;
        }

        // Maps are typically located in /Content/Maps/. The URL should be the map name itself if it's in a registered map path.
        // Or use the full path /Game/Maps/YourMapName
        FString URL = FString::Printf(TEXT("/Game/Maps/%s%s?listen"), *SelectedMapName, *GameModePathOption);
        UE_LOG(LogTemp, Log, TEXT("Attempting to open level with URL: %s"), *URL);

        UGameplayStatics::OpenLevel(GetWorld(), FName(*URL), true); // true for Absolute Travel as host

        if (MenuManager) MenuManager->ToggleMainMenu(); // Close menu after starting
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create session. SessionName: %s"), *SessionName.ToString());
        // Optionally, inform the player via UI
    }
}