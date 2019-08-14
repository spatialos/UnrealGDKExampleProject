#include "ExampleProjectTestsModule.h"
#include "Modules/ModuleManager.h"
#include "SpatialAutomationCommon.h"
//#include "../SpatialAutomationCommon.h"
#include "Misc/AutomationTest.h"
#include "SpatialAutomationTest.h"

DEFINE_LOG_CATEGORY(LogSpatialAutomationTest);

void FExampleProjectTestsModule::StartupModule()
{
}

void FExampleProjectTestsModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FExampleProjectTestsModule, ExampleProjectTests);
