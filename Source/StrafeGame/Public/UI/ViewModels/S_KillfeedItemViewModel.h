#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "S_KillfeedItemViewModel.generated.h"

class UTexture2D;
class APlayerController;

// Define FKillfeedEventData HERE - THIS IS THE SINGLE DEFINITION
USTRUCT(BlueprintType)
struct FKillfeedEventData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Killfeed")
    FString KillerName;

    UPROPERTY(BlueprintReadOnly, Category = "Killfeed")
    FString VictimName;

    UPROPERTY(BlueprintReadOnly, Category = "Killfeed")
    TSoftObjectPtr<UTexture2D> WeaponIcon;

    UPROPERTY(BlueprintReadOnly, Category = "Killfeed")
    bool bIsLocalPlayerKiller;

    UPROPERTY(BlueprintReadOnly, Category = "Killfeed")
    bool bIsLocalPlayerVictim;

    UPROPERTY(BlueprintReadOnly, Category = "Killfeed") // Ensured this is present
        bool bWasSuicide;

    FKillfeedEventData()
        : bIsLocalPlayerKiller(false)
        , bIsLocalPlayerVictim(false)
        , bWasSuicide(false)
    {
    }
};


UCLASS(BlueprintType)
class STRAFEGAME_API US_KillfeedItemViewModel : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Killfeed Item ViewModel")
    FString KillerName;

    UPROPERTY(BlueprintReadOnly, Category = "Killfeed Item ViewModel")
    FString VictimName;

    UPROPERTY(BlueprintReadOnly, Category = "Killfeed Item ViewModel")
    TSoftObjectPtr<UTexture2D> WeaponIcon;

    UPROPERTY(BlueprintReadOnly, Category = "Killfeed Item ViewModel")
    bool bIsLocalPlayerKiller;

    UPROPERTY(BlueprintReadOnly, Category = "Killfeed Item ViewModel")
    bool bIsLocalPlayerVictim;

    UPROPERTY(BlueprintReadOnly, Category = "Killfeed Item ViewModel") // Ensured this is present
        bool bWasSuicide;

    void Initialize(const FKillfeedEventData& KillData, APlayerController* LocalPlayerControllerContext);
};