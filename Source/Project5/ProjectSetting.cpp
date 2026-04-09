#include "ProjectSetting.h"

#if WITH_EDITOR
UProjectSetting::UProjectSetting()
{}
void UProjectSetting::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	// Add logic here if needed
}
#endif