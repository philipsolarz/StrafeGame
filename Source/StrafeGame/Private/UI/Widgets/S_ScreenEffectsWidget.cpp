#include "UI/Widgets/S_ScreenEffectsWidget.h"
#include "UI/ViewModels/S_ScreenEffectsViewModel.h"
#include "Components/Image.h" // Or Components/Border.h

void US_ScreenEffectsWidget::SetViewModel(US_ScreenEffectsViewModel* InViewModel)
{
    if (ScreenEffectsViewModel)
    {
        ScreenEffectsViewModel->OnScreenEffectsChanged.RemoveDynamic(this, &US_ScreenEffectsWidget::HandleViewModelUpdated);
    }
    ScreenEffectsViewModel = InViewModel;
    if (ScreenEffectsViewModel)
    {
        ScreenEffectsViewModel->OnScreenEffectsChanged.AddUniqueDynamic(this, &US_ScreenEffectsWidget::HandleViewModelUpdated);
        RefreshWidget();
    }
    else
    {
        if (DamageIndicatorImage) DamageIndicatorImage->SetOpacity(0.f);
    }
}

void US_ScreenEffectsWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (ScreenEffectsViewModel) RefreshWidget();
    else if (DamageIndicatorImage) DamageIndicatorImage->SetOpacity(0.f); // Default to hidden
}

void US_ScreenEffectsWidget::NativeDestruct()
{
    if (ScreenEffectsViewModel)
    {
        ScreenEffectsViewModel->OnScreenEffectsChanged.RemoveDynamic(this, &US_ScreenEffectsWidget::HandleViewModelUpdated);
    }
    Super::NativeDestruct();
}

void US_ScreenEffectsWidget::HandleViewModelUpdated()
{
    RefreshWidget();
}

void US_ScreenEffectsWidget::RefreshWidget()
{
    if (!ScreenEffectsViewModel || !DamageIndicatorImage) return;

    // Example: Control opacity of a red border/image based on intensity
    DamageIndicatorImage->SetOpacity(ScreenEffectsViewModel->DamageEffectIntensity);
    // You could also change its color tint here, e.g., make it more red.
    // DamageIndicatorImage->SetColorAndOpacity(FLinearColor(1.f, 0.f, 0.f, ScreenEffectsViewModel->DamageEffectIntensity));
}