#include "MessageBusPrivatePCH.h"

#define LOCTEXT_NAMESPACE "FMessageBusModule"

void FMessageBusModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FMessageBusModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMessageBusModule, MessageBus)

DEFINE_LOG_CATEGORY(LogMessageBus);
