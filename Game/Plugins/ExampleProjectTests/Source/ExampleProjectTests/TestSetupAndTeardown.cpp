#pragma once
#include "ExampleProjectTests.h"
#include "AutomationCommon.h"
#include "Editor/UnrealEd/Public/Tests/AutomationEditorCommon.h"
#include "Kismet/GameplayStatics.h"
#include "CoreMinimal.h"
#include "Controllers/GDKPlayerController.h"
#include "SpatialAutomationCommon.h"
#include "SpatialNetDriver.h"
#include "GDKCharacter.h"
#include "Interop/SpatialStaticComponentView.h"
#include "TestingObject.h"
#include "TestLevelScriptActor.h"
#include "TestingConstants.h"


#if WITH_DEV_AUTOMATION_TESTS

// Names of tests determine the order they're run in. It runs tests in alphabetic order, and tests in subfolders (Spatial.<subfolder>.<test>) before tests outside of subfolders (Spatial.<test>).
// This naming is intended to make sure that Spatial runtime is always started before any other test, and stopped after all other tests have finished.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestSetup, "Spatial.A.StartSpatial", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ServerContext)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestTeardown, "Spatial.StopSpatial", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ServerContext)

SpatialAutomationCommon::SpatialProcessInfo ProcInfo;

bool FTestSetup::RunTest(const FString& Parameters)
{
	FString WorkerConfigFile = TEXT("default_launch.json");
	bool Started = SpatialAutomationCommon::StartSpatial(this, ProcInfo, WorkerConfigFile);

	return true;
}

bool FTestTeardown::RunTest(const FString& Parameters)
{
	ADD_LATENT_AUTOMATION_COMMAND(FStopLocalSpatialGame(ProcInfo));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
