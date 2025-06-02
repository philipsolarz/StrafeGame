#include "UI/ViewModels/S_ScreenEffectsViewModel.h"
#include "Player/S_PlayerController.h"
#include "Player/S_PlayerState.h"
#include "Player/Attributes/S_AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

void US_ScreenEffectsViewModel::Initialize(AS_PlayerController* InOwningPlayerController)
{
    Super::Initialize(InOwningPlayerController);
    DamageEffectIntensity = 0.0f;

    AS_PlayerState* PS = GetOwningPlayerController() ? GetOwningPlayerController()->GetPlayerState<AS_PlayerState>() : nullptr;
    if (PS)
    {
        PlayerAttributeSet = PS->GetAttributeSet();
        PlayerAbilitySystemComponent = PS->GetAbilitySystemComponent();

        if (PlayerAbilitySystemComponent.IsValid() && PlayerAttributeSet.IsValid())
        {
            HealthChangedDelegateHandle = PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributeSet->GetHealthAttribute()).AddUObject(this, &US_ScreenEffectsViewModel::HandleHealthChanged);
            LastNotifiedHealth = PlayerAttributeSet->GetHealth();
        }
    }
}

void US_ScreenEffectsViewModel::Deinitialize()
{
    if (PlayerAbilitySystemComponent.IsValid() && PlayerAttributeSet.IsValid())
    {
        PlayerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(PlayerAttributeSet->GetHealthAttribute()).Remove(HealthChangedDelegateHandle);
    }
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(DamageEffectResetTimerHandle);
    }
    Super::Deinitialize();
}

void US_ScreenEffectsViewModel::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
    if (Data.NewValue < LastNotifiedHealth)
    {
        TriggerDamageEffect(LastNotifiedHealth - Data.NewValue);
    }
    LastNotifiedHealth = Data.NewValue;
}

void US_ScreenEffectsViewModel::TriggerDamageEffect(float DamageAmount)
{
    // Example: Intensity based on damage, capped at 1.0
    // More sophisticated logic can be used here (e.g. based on % health lost)
    DamageEffectIntensity = FMath::Clamp(DamageEffectIntensity + DamageAmount * 0.02f, 0.0f, 1.0f); // Small multiplier
    OnScreenEffectsChanged.Broadcast();

    if (GetWorld())
    {
        // Reset the effect after a short duration
        GetWorld()->GetTimerManager().SetTimer(DamageEffectResetTimerHandle, this, &US_ScreenEffectsViewModel::ResetDamageEffect, 0.25f, false);
    }
}

void US_ScreenEffectsViewModel::ResetDamageEffect()
{
    DamageEffectIntensity = 0.0f;
    OnScreenEffectsChanged.Broadcast();
}