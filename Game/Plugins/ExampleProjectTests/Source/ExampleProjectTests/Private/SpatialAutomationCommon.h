#pragma once

#include "AutomationEditorCommon.h"
#include "AutomationCommon.h"
#include "CoreMinimal.h"
#include "LogMacros.h"

#if WITH_DEV_AUTOMATION_TESTS

#define SPATIAL_TEST(TestName, FeatureName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestName, "Spatial.Core."#FeatureName"."#TestName, EAutomationTestFlags::EngineFilter | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext) \
	bool TestName::RunTest(const FString& Parameters)

#define LATENT_TEST_COMMAND_ONE_PARAMETER(CommandName, TestName) \
	DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(CommandName, FAutomationTestBase*, TestName); \
	bool CommandName::Update()

#define ADD_LATENT_AUTOMATION_COMMAND_WITH_DELAY(ClassDeclaration, Delay) \
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(Delay)); \
	ADD_LATENT_AUTOMATION_COMMAND(ClassDeclaration);

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(TestIfAtLeastGivenNumberOfActorsExist, TSubclassOf<AActor>, ActorClass, int, LowerLimit, FAutomationTestBase*, Test);

class SpatialAutomationCommon
{
public:

	static const float kDefaultPlayerJoinDelay;

	static void StartPIE(float PlayerJoinDelay = kDefaultPlayerJoinDelay);

	static UWorld* GetActiveGameWorld();

	static int GetNumberOfActorsOfClass(TSubclassOf<AActor> ActorClass);

};

#endif // WITH_DEV_AUTOMATION_TESTS
