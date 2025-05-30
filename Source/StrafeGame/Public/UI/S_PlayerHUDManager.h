// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "S_PlayerHUDManager.generated.h"

class UUserWidget;
class AS_PlayerController; // Forward declaration

UCLASS()
class STRAFEGAME_API AS_PlayerHUDManager : public AActor
{
    GENERATED_BODY()

public:
    AS_PlayerHUDManager();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /**
     * Initializes the HUD manager with the owning player controller.
     * This is where the main HUD widget would be created and added to the viewport.
     * Called by AS_PlayerController after this manager is spawned.
     */
    virtual void Initialize(AS_PlayerController* OwningPlayerController);

    UFUNCTION(BlueprintPure, Category = "HUDManager")
    AS_PlayerController* GetOwningStrafePlayerController() const;

protected:
    UPROPERTY(Transient) // Owning PC doesn't need to be replicated or saved
        TObjectPtr<AS_PlayerController> OwningStrafePC;

    /** The root UMG widget for the main HUD. Will be created in Initialize. */
    UPROPERTY(BlueprintReadOnly, Category = "HUDManager|Widgets")
    TObjectPtr<UUserWidget> MainHUDWidget;

    /**
     * Class of the main HUD UMG widget to create.
     * This should be assigned in a Blueprint subclass of AS_PlayerHUDManager,
     * which is then set in the AS_PlayerController's defaults.
     */
    UPROPERTY(EditDefaultsOnly, Category = "HUDManager|Widgets", Meta = (DisplayName = "Main HUD Widget Class"))
    TSubclassOf<UUserWidget> MainHUDWidgetClass;

public:
    // Example function to be implemented in Blueprints for showing announcer messages.
    // This demonstrates how game systems can interact with the HUD via this manager.
    UFUNCTION(BlueprintImplementableEvent, Category = "HUDManager|Announcer")
    void ShowAnnouncerMessage(const FText& Message, float Duration);
};