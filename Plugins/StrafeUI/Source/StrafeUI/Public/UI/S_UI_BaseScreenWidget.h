// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_BaseScreenWidget.h

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "ViewModel/S_UI_ViewModelBase.h"
#include "S_UI_BaseScreenWidget.generated.h"

class US_UI_Subsystem;
struct FInputActionValue;

/**
 * @class S_UI_BaseScreenWidget
 * @brief A base class for all primary UI screens in the game.
 * Provides common functionality, such as easy access to the UI Subsystem.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_BaseScreenWidget : public UCommonActivatableWidget, public IViewModelProvider
{
    GENERATED_BODY()

public:
    /**
     * @brief Handles the navigate input action. Override in child classes to implement navigation logic.
     * @param NavDirection The 2D navigation vector.
     */
    virtual void HandleNavigation(const FVector2D& NavDirection) {}

    /**
     * @brief Handles the accept/confirm input action. Override in child classes.
     */
    virtual void HandleAccept() {}

    /**
     * When implemented by a widget, this function is responsible for creating and returning
     * the specific ViewModel instance that the widget will use.
     * @return A pointer to the newly created ViewModel.
     */
    virtual US_UI_ViewModelBase* CreateViewModel() override { return nullptr; };


protected:
    /**
     * @brief Gets the UI Subsystem.
     * @return A pointer to the US_UI_Subsystem.
     */
    UFUNCTION(BlueprintCallable, Category = "UI")
    US_UI_Subsystem* GetUISubsystem() const;
};