#include "DynamicStorageProjectSetting.h"

UDynamicStorageProjectSetting::UDynamicStorageProjectSetting()
{}
#if WITH_EDITOR
void UDynamicStorageProjectSetting::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	// Add logic here if needed
}
#endif