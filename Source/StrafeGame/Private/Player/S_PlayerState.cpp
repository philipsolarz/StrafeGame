#include "Player/S_PlayerState.h"
#include "Player/S_Character.h"
#include "Player/Attributes/S_AttributeSet.h" // To be created
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h" // Ensure UGameplayEffect is fully included
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"

AS_PlayerState::AS_PlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    AttributeSet = CreateDefaultSubobject<US_AttributeSet>(TEXT("AttributeSet_Primary"));

    PrimaryActorTick.bCanEverTick = false;
    bIsDead = false;
}

void AS_PlayerState::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        InitializeAttributes();
        GrantDefaultAbilities();

        if (AbilitySystemComponent && AttributeSet)
        {
            HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
                .AddUObject(this, &AS_PlayerState::HandleHealthChange);
        }
    }
}

void AS_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(AS_PlayerState, bIsDead, COND_None, REPNOTIFY_Always);
}

void AS_PlayerState::Reset()
{
    Super::Reset();
    bIsDead = false;
    // Server should re-initialize attributes and abilities for a clean state
    if (HasAuthority())
    {
        InitializeAttributes();
        GrantDefaultAbilities();
    }
    // OnRep_IsDead will handle client-side visual reset if pawn exists
}

void AS_PlayerState::ClientInitialize(AController* C)
{
    Super::ClientInitialize(C);
    if (AbilitySystemComponent)
    {
        AS_Character* SChar = nullptr;
        if (C) SChar = Cast<AS_Character>(C->GetPawn());
        if (SChar) AbilitySystemComponent->InitAbilityActorInfo(this, SChar);
    }
    if (AbilitySystemComponent && AttributeSet && !HealthChangedDelegateHandle.IsValid())
    {
        HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
            .AddUObject(this, &AS_PlayerState::HandleHealthChange);
    }
}

void AS_PlayerState::CopyProperties(APlayerState* InPlayerState)
{
    Super::CopyProperties(InPlayerState);
    // AS_PlayerState* SourcePlayerState = Cast<AS_PlayerState>(InPlayerState);
    // if (SourcePlayerState)
    // {
    //      this->bIsDead = SourcePlayerState->bIsDead; // Though death state might not typically be copied
    // }
}

UAbilitySystemComponent* AS_PlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AS_PlayerState::InitializeAttributes()
{
    if (!AbilitySystemComponent || !AttributeSet || !DefaultAttributesEffect) return; // MODIFIED: Check if class is set, not if pointer is valid
    if (HasAuthority())
    {
        FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
        EffectContext.AddSourceObject(this);
        // MODIFIED: Directly use the TSubclassOf<UGameplayEffect>
        FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributesEffect, 1, EffectContext);
        if (SpecHandle.IsValid())
        {
            AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AS_PlayerState::InitializeAttributes: Failed to create spec for DefaultAttributesEffect %s."), *DefaultAttributesEffect->GetName());
        }
    }
}

void AS_PlayerState::GrantDefaultAbilities()
{
    if (!AbilitySystemComponent || !HasAuthority()) return;
    for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultPlayerAbilities)
    {
        if (AbilityClass)
        {
            FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
            AbilitySystemComponent->GiveAbility(AbilitySpec);
        }
    }
}

void AS_PlayerState::HandleHealthChange(const FOnAttributeChangeData& Data)
{
    if (bIsDead) return;
    if (Data.NewValue <= 0.f && Data.OldValue > 0.f)
    {
        if (HasAuthority())
        {
            ServerProcessPlayerDeath();
        }
    }
}

void AS_PlayerState::ServerProcessPlayerDeath()
{
    if (!HasAuthority() || bIsDead) return;
    bIsDead = true; // Set death flag
    OnRep_IsDead(); // Call OnRep manually on server to trigger logic immediately

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.Dead")));
    }
    NotifyPlayerDeath();
}

void AS_PlayerState::OnRep_IsDead()
{
    // This is called on clients when bIsDead changes.
    // And manually on server via ServerProcessPlayerDeath.
    // If the player is now marked as dead, but wasn't before locally:
    AS_Character* MyCharacter = GetSCharacter();
    if (MyCharacter && bIsDead) // Check bIsDead directly
    {
        // If the character isn't already "visually" dead, make it so.
        // MyCharacter->HandleDeath(); // This could be called here,
                                  // or HandleDeath could be more focused on server-side state
                                  // and visuals are driven by a GameplayCue.
                                  // For now, direct call is fine if HandleDeath is idempotent on client.
    }
    // If !bIsDead, it means player respawned, character might need visual reset if HandleDeath wasn't fully clean.
}


void AS_PlayerState::NotifyPlayerDeath()
{
    AS_Character* MyCharacter = GetSCharacter();
    if (MyCharacter)
    {
        MyCharacter->HandleDeath();
    }
    OnPlayerDiedDelegate.Broadcast(this);
}

AS_Character* AS_PlayerState::GetSCharacter() const
{
    return GetPawn<AS_Character>();
}