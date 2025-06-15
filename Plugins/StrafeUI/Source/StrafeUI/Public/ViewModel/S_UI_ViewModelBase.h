// Plugins/StrafeUI/Source/StrafeUI/Public/ViewModel/S_UI_ViewModelBase.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/Object.h"
#include "S_UI_ViewModelBase.generated.h"

/**
 * Delegate to broadcast when view model data changes.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDataChanged);

UINTERFACE(MinimalAPI, Blueprintable)
class UViewModelProvider : public UInterface
{
	GENERATED_BODY()
};

class IViewModelProvider
{
	GENERATED_BODY()

public:
	/**
	 * When implemented by a widget, this function is responsible for creating and returning
	 * the specific ViewModel instance that the widget will use.
	 * @return A pointer to the newly created ViewModel.
	 */
	virtual class US_UI_ViewModelBase* CreateViewModel() = 0;
};


/**
 * @class S_UI_ViewModelBase
 * @brief Abstract base class for all ViewModels in the StrafeUI plugin.
 *
 * This class provides the fundamental contract for how ViewModels notify listening systems
 * of state changes. It is designed to be inherited by all specific ViewModel classes.
 */
UCLASS(BlueprintType, Abstract)
class STRAFEUI_API US_UI_ViewModelBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Broadcasts a notification that the ViewModel's data has changed.
	 * Call this function whenever a property that the View is bound to is updated.
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewModel")
	void BroadcastDataChanged();

	/**
	 * Delegate that is broadcast whenever the ViewModel's data changes.
	 * Views can bind to this delegate to receive notifications and update themselves accordingly.
	 */
	UPROPERTY(BlueprintAssignable, Category = "ViewModel")
	FOnDataChanged OnDataChanged;
};