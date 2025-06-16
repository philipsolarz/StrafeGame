// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_StringComboBox.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ComboBoxString.h"
#include "S_UI_StringComboBox.generated.h"

/**
 * A simple wrapper around UComboBoxString to make it easier to use in our MVVM pattern.
 */
UCLASS()
class STRAFEUI_API US_UI_StringComboBox : public UComboBoxString
{
    GENERATED_BODY()
};