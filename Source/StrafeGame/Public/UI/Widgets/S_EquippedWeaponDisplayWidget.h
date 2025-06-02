#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S_EquippedWeaponDisplayWidget.generated.h"

class US_PlayerHUDViewModel;
class US_WeaponViewModel;
class UWidgetSwitcher;
class US_ChargedShotgunStatusWidget; // Forward declare C++ widget type
class US_StickyLauncherStatusWidget; // Forward declare C++ widget type

UCLASS()
class STRAFEGAME_API US_EquippedWeaponDisplayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Equipped Weapon Display")
    void SetPlayerHUDViewModel(US_PlayerHUDViewModel* InViewModel);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "ViewModel")
    TObjectPtr<US_PlayerHUDViewModel> PlayerHUDViewModel;

    // This will be bound to the WidgetSwitcher in the WBP_EquippedWeaponDisplay Blueprint
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) // Optional if not always present
        TObjectPtr<UWidgetSwitcher> WeaponSpecificSwitcher;

    // You'll need to create these UMG child widgets in the WBP_EquippedWeaponDisplay
    // and assign them to the WidgetSwitcher. Then, get references to them here if needed,
    // or create them dynamically in C++. For simplicity, let's assume they are added
    // as children to the switcher in the UMG editor.
    // We'll primarily manage the active index of the switcher.

    // Store references to the actual C++ widget instances if created/managed here
    UPROPERTY()
    TObjectPtr<US_ChargedShotgunStatusWidget> ChargedShotgunStatusInstance;
    UPROPERTY()
    TObjectPtr<US_StickyLauncherStatusWidget> StickyLauncherStatusInstance;
    // Add more for other weapon types

    // Classes for dynamic creation if preferred
    UPROPERTY(EditDefaultsOnly, Category = "Equipped Weapon Display|WidgetClasses")
    TSubclassOf<US_ChargedShotgunStatusWidget> ChargedShotgunStatusWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "EquippedWeapon Display|WidgetClasses")
    TSubclassOf<US_StickyLauncherStatusWidget> StickyLauncherStatusWidgetClass;


    UFUNCTION()
    virtual void HandleViewModelUpdated();

    virtual void UpdateEquippedWeaponDisplay();

private:
    // Helper to ensure a specific widget type is created and in the switcher
    UUserWidget* GetOrCreateWeaponSpecificWidget(TSubclassOf<UUserWidget> WidgetClass, int32& OutWidgetIndex);
};