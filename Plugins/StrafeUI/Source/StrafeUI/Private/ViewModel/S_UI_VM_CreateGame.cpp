// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_VM_CreateGame.cpp

#include "ViewModel/S_UI_VM_CreateGame.h"
#include "S_UI_Settings.h"
#include "S_UI_Subsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Data/S_UI_ScreenTypes.h"
#include "StrafeMultiplayer/Public/StrafeMultiplayerSubsystem.h"
#include "StrafeMultiplayer/Public/MultiplayerSessionTypes.h"

void US_UI_VM_CreateGame::Initialize(const US_UI_Settings* InSettings)
{
	UISettings = InSettings;
	if (!UISettings.IsValid())
	{
		return;
	}

	// Populate the game mode display names from the settings
	for (const FStrafeGameModeInfo& GameModeInfo : UISettings->AvailableGameModes)
	{
		GameModeDisplayNames.Add(GameModeInfo.DisplayName.ToString());
	}

	// Set the initial selection to the first available game mode
	if (GameModeDisplayNames.Num() > 0)
	{
		OnGameModeChanged(GameModeDisplayNames[0]);
	}
	else
	{
		BroadcastDataChanged(); // Ensure UI is at least cleared if no modes are defined
	}

	// Get the StrafeMultiplayer subsystem and bind delegates
	if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		MultiplayerSubsystem = GameInstance->GetSubsystem<UStrafeMultiplayerSubsystem>();
		if (MultiplayerSubsystem)
		{
			MultiplayerSubsystem->OnCreateSessionComplete.AddDynamic(this, &US_UI_VM_CreateGame::OnCreateSessionComplete);
		}
	}
}

void US_UI_VM_CreateGame::CreateGame()
{
	if (!MultiplayerSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: MultiplayerSubsystem is null"));
		return;
	}

	if (!UISettings.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: UISettings is invalid"));
		return;
	}

	// Find the full GameModeInfo struct from the selected display name
	const FStrafeGameModeInfo* SelectedGameModeInfo = UISettings->AvailableGameModes.FindByPredicate(
		[this](const FStrafeGameModeInfo& Info)
		{
			return Info.DisplayName.ToString() == SelectedGameModeName;
		});

	if (!SelectedGameModeInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: Could not find settings for selected game mode '%s'"), *SelectedGameModeName);
		return;
	}

	// Find the map asset from the selected map display name
	const TSoftObjectPtr<UWorld>* SelectedMapAsset = SelectedGameModeInfo->CompatibleMaps.FindByPredicate(
		[this](const TSoftObjectPtr<UWorld>& MapAsset)
		{
			// Get the asset name from the soft pointer path
			return FPaths::GetBaseFilename(MapAsset.ToString()) == SelectedMapName;
		});

	if (!SelectedMapAsset || SelectedMapAsset->IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: Could not find asset for selected map '%s'"), *SelectedMapName);
		return;
	}

	// Store the map path for later use
	CachedMapAssetPath = (*SelectedMapAsset).ToSoftObjectPath().GetLongPackageName();

	// Create session settings for the StrafeMultiplayer subsystem
	FStrafeGameSessionSettings SessionSettings;
	SessionSettings.MaxPlayers = MaxPlayers;
	SessionSettings.GameMode = SelectedGameModeName;
	SessionSettings.MapName = SelectedMapName;
	SessionSettings.GameModeSettings = FString::Printf(TEXT("TimeLimit=%d,ScoreLimit=%d"), TimeLimit, ScoreLimit);
	SessionSettings.bIsLANMatch = bIsLANMatch;
	SessionSettings.bIsDedicated = bIsDedicatedServer;

	UE_LOG(LogTemp, Log, TEXT("Creating game session with StrafeMultiplayer subsystem"));
	MultiplayerSubsystem->CreateSession(SessionSettings);
}

void US_UI_VM_CreateGame::OnCreateSessionComplete(EMultiplayerSessionResult Result)
{
	if (Result == EMultiplayerSessionResult::Success)
	{
		UE_LOG(LogTemp, Log, TEXT("Session created successfully. Traveling to map..."));

		// Travel to the map
		if (UWorld* World = GetWorld())
		{
			World->ServerTravel(CachedMapAssetPath + "?listen");
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create session. Result: %d"), (int32)Result);

		// Show error modal
		if (UWorld* World = GetWorld())
		{
			if (US_UI_Subsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
			{
				F_UIModalPayload Payload;
				Payload.Message = FText::FromString(TEXT("Failed to create game session. Please check your connection and try again."));
				Payload.ModalType = E_UIModalType::OK;
				UISubsystem->RequestModal(Payload, FOnModalDismissedSignature());
			}
		}
	}
}

void US_UI_VM_CreateGame::OnGameModeChanged(FString InSelectedGameModeName)
{
	SelectedGameModeName = InSelectedGameModeName;
	UE_LOG(LogTemp, Warning, TEXT("[CreateGameVM] OnGameModeChanged: ViewModel's game mode is now '%s'. Broadcasting change."), *SelectedGameModeName);
	MapDisplayNames.Empty();

	if (!UISettings.IsValid()) return;

	// Find the game mode info struct that matches the new selection
	const FStrafeGameModeInfo* GameModeInfo = UISettings->AvailableGameModes.FindByPredicate(
		[this](const FStrafeGameModeInfo& Info)
		{
			return Info.DisplayName.ToString() == SelectedGameModeName;
		});

	// If found, populate the map display names from its compatible maps list
	if (GameModeInfo)
	{
		for (const TSoftObjectPtr<UWorld>& MapAsset : GameModeInfo->CompatibleMaps)
		{
			MapDisplayNames.Add(FPaths::GetBaseFilename(MapAsset.ToString()));
		}
	}

	// Default to the first map in the list if available
	if (MapDisplayNames.Num() > 0)
	{
		SelectedMapName = MapDisplayNames[0];
	}

	BroadcastDataChanged();
}