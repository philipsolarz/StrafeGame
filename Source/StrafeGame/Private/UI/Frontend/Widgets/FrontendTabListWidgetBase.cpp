// Philip Solarz All Rights Reserved


#include "UI/Frontend/Widgets/FrontendTabListWidgetBase.h"
#include "Editor/WidgetCompilerLog.h"
#include "UI/Frontend/Widgets/FrontendCommonButtonBase.h"

#if WITH_EDITOR	
void UFrontendTabListWidgetBase::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	if (!TabButtonEntryWidgetClass)
	{
		CompileLog.Error(FText::FromString(
			TEXT("The variable TabButtonEntryWidgetClass has no valid entry specified. ") +
			GetClass()->GetName() +
			TEXT(" needs a valid entry widget class to function properly")
		));
	}
}
#endif