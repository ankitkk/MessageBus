#pragma once

#include "ModuleManager.h"

class FMessageBusModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

DECLARE_LOG_CATEGORY_EXTERN(LogMessageBus, VeryVerbose, All);