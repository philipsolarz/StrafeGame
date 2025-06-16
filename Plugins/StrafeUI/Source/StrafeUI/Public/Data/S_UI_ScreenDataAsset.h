// Plugins/StrafeUI/Source/StrafeUI/Public/Data/S_UI_ScreenDataAsset.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/S_UI_ScreenTypes.h"
#include "S_UI_ScreenDataAsset.generated.h"

/**
 * @class US_UI_ScreenDataAsset
 * @brief A C++ defined Primary Data Asset that holds an array of Screen Definitions.
 * Your Blueprint Data Asset (DA_ScreenMap) should be parented to this class.
 */
UCLASS()
class STRAFEUI_API US_UI_ScreenDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** The list of screen definitions that map a Screen ID to a Widget Class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Screens")
	TArray<F_UIScreenDefinition> ScreenDefinitions;
};