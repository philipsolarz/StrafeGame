#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/S_ViewModelBase.h"
#include "GameplayEffectTypes.h" // For FOnAttributeChangeData
#include "S_ScreenEffectsViewModel.generated.h"

class US_AttributeSet;
class UAbilitySystemComponent;

UCLASS()
class STRAFEGAME_API US_ScreenEffectsViewModel : public US_ViewModelBase
{
    GENERATED_BODY()

public:
    virtual void Initialize(AS_PlayerController* InOwningPlayerController) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintReadOnly, Category = "ScreenEffectsViewModel")
    float DamageEffectIntensity; // 0.0 (no effect) to 1.0 (full effect)

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnScreenEffectsChanged);
    UPROPERTY(BlueprintAssignable, Category = "ScreenEffectsViewModel|Events")
    FOnScreenEffectsChanged OnScreenEffectsChanged;

protected:
    TWeakObjectPtr<US_AttributeSet> PlayerAttributeSet;
    TWeakObjectPtr<UAbilitySystemComponent> PlayerAbilitySystemComponent;
    FDelegateHandle HealthChangedDelegateHandle;

    float LastNotifiedHealth;

    virtual void HandleHealthChanged(const FOnAttributeChangeData& Data);
    void TriggerDamageEffect(float DamageAmount);
    void ResetDamageEffect();

    FTimerHandle DamageEffectResetTimerHandle;
};