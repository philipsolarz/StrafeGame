#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"    // For IAbilitySystemInterface
#include "GameplayEffectTypes.h"       // For FOnAttributeChangeData
#include "Delegates/DelegateCombinations.h"
#include "S_PlayerState.generated.h"

// Forward Declarations
class UAbilitySystemComponent;
class US_AttributeSet; // Your primary attribute set
class UGameplayEffect;
class UGameplayAbility;
class AS_Character;

// Delegate for when this PlayerState's controlled character dies
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateDiedDelegate, AS_PlayerState*, DeadPlayerState);

UCLASS(Blueprintable, Config = Game)
class STRAFEGAME_API AS_PlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AS_PlayerState();

    //~ Begin AActor Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    //~ End AActor Interface

    //~ Begin APlayerState Interface
    virtual void Reset() override;
    virtual void ClientInitialize(AController* C) override;
    virtual void CopyProperties(APlayerState* PlayerState) override;
    //~ End APlayerState Interface

    //~ Begin IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    //~ End IAbilitySystemInterface

    /** Getter for the primary AttributeSet. */
    UFUNCTION(BlueprintPure, Category = "PlayerState|GAS")
    US_AttributeSet* GetAttributeSet() const { return AttributeSet; }

    /** Delegate broadcast when this player's controlled character dies (Health <= 0). */
    UPROPERTY(BlueprintAssignable, Category = "PlayerState|Events")
    FOnPlayerStateDiedDelegate OnPlayerDiedDelegate;

    /** Helper to get the controlled S_Character. */
    UFUNCTION(BlueprintPure, Category = "PlayerState")
    AS_Character* GetSCharacter() const;

    /** Called by the server when pawn health reaches zero. */
    virtual void NotifyPlayerDeath();

    UFUNCTION(BlueprintPure, Category = "PlayerState|Status")
    bool IsPlayerDead() const { return bIsDead; }

protected:
    /** The AbilitySystemComponent for this PlayerState. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerState|GAS", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    /** The primary AttributeSet for this PlayerState. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerState|GAS", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<US_AttributeSet> AttributeSet;

    // --- GAS DATA ASSETS & INITIALIZATION ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerState|GAS|Defaults", meta = (DisplayName = "Player Default Attributes Effect"))
    TSoftObjectPtr<UGameplayEffect> DefaultAttributesEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerState|GAS|Defaults", meta = (DisplayName = "Default Player-Owned Abilities"))
    TArray<TSubclassOf<UGameplayAbility>> DefaultPlayerAbilities;

    virtual void InitializeAttributes();
    virtual void GrantDefaultAbilities();

    // --- ATTRIBUTE CHANGE HANDLING ---
    FDelegateHandle HealthChangedDelegateHandle;

    virtual void HandleHealthChange(const FOnAttributeChangeData& Data);

    /** Server-side function that gets called when health reaches zero. Triggers death process. */
    virtual void ServerProcessPlayerDeath();

    /** Flag to prevent processing death multiple times. Replicated. */
    UPROPERTY(Transient, ReplicatedUsing = OnRep_IsDead)
    bool bIsDead;

    UFUNCTION()
    virtual void OnRep_IsDead();
};