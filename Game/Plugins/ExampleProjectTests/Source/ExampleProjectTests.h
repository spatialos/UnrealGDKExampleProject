#pragma once

#include "Modules/ModuleInterface.h"

class FExampleProjectTests : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void StartSpatialProcessBeforeTesting();
	void StopSpatialProcessAfterTesting();
};
