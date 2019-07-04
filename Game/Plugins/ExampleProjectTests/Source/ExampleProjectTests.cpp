#include "ExampleProjectTests.h"
#include "Modules/ModuleManager.h"
#include "ExampleProjectTests/SpatialAutomationCommon.h"
#include "Misc/AutomationTest.h"

void FExampleProjectTests::StartupModule()
{
	// Note: PreTestingEvent and PostTestingEvent probably aren't what I think they are.
	//UE_LOG(LogTemp, Warning, TEXT("At test module startup, adding Spatial runtime bindings for before and after tests."));
	//FAutomationTestFramework::Get().PreTestingEvent.AddRaw(this, &FExampleProjectTests::StartSpatialProcessBeforeTesting);
	//FAutomationTestFramework::Get().PostTestingEvent.AddRaw(this, &FExampleProjectTests::StopSpatialProcessAfterTesting);
}

void FExampleProjectTests::ShutdownModule()
{
	UE_LOG(LogTemp, Warning, TEXT("At test module shutdown."));
}

void FExampleProjectTests::StartSpatialProcessBeforeTesting()
{
	UE_LOG(LogTemp, Warning, TEXT("At StartSpatialProcessBeforeTesting, Starting Spatial Process"));
	FString WorkerConfigFile = TEXT("default_launch.json");
	SpatialAutomationCommon::SpatialProcessInfo ProcInfo;
	bool Started = SpatialAutomationCommon::StartSpatialAndPIE(ProcInfo, WorkerConfigFile);
	// TODO if Spatial fails stop testing
}

void FExampleProjectTests::StopSpatialProcessAfterTesting()
{
	UE_LOG(LogTemp, Warning, TEXT("At StopSpatialProcessAfterTesting, Stopping Spatial Process"));
	SpatialAutomationCommon::SpatialProcessInfo ProcInfo;
	SpatialAutomationCommon::StopCurrentSpatialProcess(ProcInfo);
}

IMPLEMENT_MODULE(FExampleProjectTests, ExampleProjectTests);
