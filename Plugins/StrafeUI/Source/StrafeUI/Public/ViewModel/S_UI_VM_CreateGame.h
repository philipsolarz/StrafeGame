// Plugins/StrafeUI/Source/StrafeUI/Public/ViewModel/S_UI_VM_CreateGame.h

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/S_UI_ViewModelBase.h"
#include "StrafeMultiplayer/Public/MultiplayerSessionTypes.h"
#include "S_UI_VM_CreateGame.generated.h"

class US_UI_Settings;
class UStrafeMultiplayerSubsystem;
struct FStrafeGameModeInfo;

UCLASS(BlueprintType)
class STRAFEUI_API US_UI_VM_CreateGame : public US_UI_ViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(const US_UI_Settings* InSettings);

	UFUNCTION(BlueprintCallable, Category = "Create Game")
	void CreateGame();

	UFUNCTION(BlueprintCallable, Category = "Create Game")
	void OnGameModeChanged(FString SelectedGameModeName);

	/** List of game mode names to display in the UI. */
	UPROPERTY(BlueprintReadOnly, Category = "Create Game")
	TArray<FString> GameModeDisplayNames;

	/** List of map names to display in the UI for the selected game mode. */
	UPROPERTY(BlueprintReadOnly, Category = "Create Game")
	TArray<FString> MapDisplayNames;

	// Basic Game Settings
	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	FString GameName = "My Awesome Game";

	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	FString ServerDescription = "Welcome to my server!";

	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	bool bIsLANMatch = false;

	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	bool bIsDedicatedServer = false;

	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	int32 MaxPlayers = 8;

	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	FString Password;

	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	FString SelectedMapName;

	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	FString SelectedGameModeName;

	// Advanced Game Settings
	UPROPERTY(BlueprintReadWrite, Category = "Create Game|Advanced")
	bool bAllowFriendlyFire = false;

	UPROPERTY(BlueprintReadWrite, Category = "Create Game|Advanced")
	bool bAllowSpectators = true;

	UPROPERTY(BlueprintReadWrite, Category = "Create Game|Advanced")
	int32 TimeLimit = 20; // in minutes

	UPROPERTY(BlueprintReadWrite, Category = "Create Game|Advanced")
	int32 ScoreLimit = 50;

	UPROPERTY(BlueprintReadWrite, Category = "Create Game|Advanced")
	float RespawnTime = 5.0f;

private:
	/** Callback for when session creation completes */
	UFUNCTION()
	void OnCreateSessionComplete(EMultiplayerSessionResult Result);

	UPROPERTY()
	TWeakObjectPtr<const US_UI_Settings> UISettings;

	UPROPERTY()
	TObjectPtr<UStrafeMultiplayerSubsystem> MultiplayerSubsystem;

	/** Cached map asset path for session creation */
	FString CachedMapAssetPath;
};