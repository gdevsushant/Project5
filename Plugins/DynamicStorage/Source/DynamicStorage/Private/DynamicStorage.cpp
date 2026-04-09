#include "DynamicStorage.h"

#define LOCTEXT_NAMESPACE "FDynamicStorageModule"

void FDynamicStorageModule::StartupModule()
{
    // This code will execute after your module is loaded into memory
}

void FDynamicStorageModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDynamicStorageModule, DynamicStorage)