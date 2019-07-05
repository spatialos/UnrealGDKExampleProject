#pragma once

#include "ExampleProjectTests.h"

#include "AutomationEditorCommon.h"
#include "AutomationCommon.h"
#include "CoreMinimal.h"
#include "LogMacros.h"

#if WITH_DEV_AUTOMATION_TESTS

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialAutomationTest, Log, All);

class SpatialAutomationCommon
{
public:
	struct SpatialProcessInfo
	{
		SpatialProcessInfo()
			: ProcessHandle()
			, ProcessID{ 0 } // PID = 0 is used for error states in CreateProc
			, ReadPipe{ nullptr }
			, WritePipe{ nullptr }
		{
		}

		FProcHandle ProcessHandle;
		uint32 ProcessID;
		void* ReadPipe;
		void* WritePipe;
	};

	static const float kDefaultSpatialStartDelay;
	static const float kDefaultPlayerJoinDelay;

	/**
	 * Start spatial.exe
	 * @param Test				The test that is being executed and will have the asserts called on
	 * @param OutProcessInfo 	ProcessInfo for the spatial.exe
	 * @param WorkerConfigFile  The configuration file to be loaded.
	 * @return					Whether spatial started successfully.
	 */
	static bool StartLocalSpatialDeploy(FAutomationTestBase* Test, SpatialProcessInfo& OutProcessInfo, const FString& WorkerConfigFile);

	/**
	 * Start Spatial.exe wait a little bit, then try to connect clients, wait a little bit for the characters to load.
	 * @param Test				The test that is being executed and will have the asserts called on
	 * @param OutProcessInfo 	ProcessInfo for the spatial.exe
	 * @param WorkerConfigFile  The configuration file to be loaded.
	 * @return					Whether spatial started successfully.
	 */
	static bool StartSpatialAndPIE(
		FAutomationTestBase* Test,
		SpatialProcessInfo& OutProcessInfo,
		const FString& WorkerConfigFile,
		float SpatialStartDelay = kDefaultSpatialStartDelay,
		float PlayerJoinDelay = kDefaultPlayerJoinDelay
	);

	static bool StartSpatial(
		FAutomationTestBase* Test,
		SpatialProcessInfo& OutProcessInfo,
		const FString& WorkerConfigFile,
		float SpatialStartDelay = kDefaultSpatialStartDelay
	);

	static void StartPIE(float PlayerJoinDelay = kDefaultPlayerJoinDelay);

	static void StartPieWithDelay(
		float SpatialStartDelay = kDefaultSpatialStartDelay,
		float PlayerJoinDelay = kDefaultPlayerJoinDelay
	);

	static UWorld* GetActiveGameWorld();

	//static void StopCurrentSpatialProcess(SpatialProcessInfo& CurrentProcessInfo);
};

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FStopLocalSpatialGame, SpatialAutomationCommon::SpatialProcessInfo, ProcessInfo);
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FTakeScreenShot, FString, FileLabel);
#endif // WITH_DEV_AUTOMATION_TESTS
