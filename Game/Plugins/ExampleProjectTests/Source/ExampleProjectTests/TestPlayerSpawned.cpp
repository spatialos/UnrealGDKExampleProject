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


#if WITH_DEV_AUTOMATION_TESTS


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestPlayerControllerGetsCreated, "Spatial.Core.PlayerControllerGetsCreated", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ServerContext)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestServerAuthoritativeOverGSM, "Spatial.Core.ServerHasAuthorityOverGSM", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ServerContext)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestObjectCanBeSpawned, "Spatial.Core.CanSpawnObjects", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ServerContext)


DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckPlayerSpawned, FTestPlayerControllerGetsCreated*, Test);
bool FCheckPlayerSpawned::Update()
{
	UWorld* World = SpatialAutomationCommon::GetActiveGameWorld();
	TArray<AActor*> PlayerController;
	UGameplayStatics::GetAllActorsOfClass(World, AGDKPlayerController::StaticClass(), PlayerController);

	Test->TestTrue("Player Controller is spawned", PlayerController.Num() >= 1);
	return true;
}

bool FTestPlayerControllerGetsCreated::RunTest(const FString& Parameters)
{
	FString WorkerConfigFile = TEXT("default_launch.json");
	SpatialAutomationCommon::SpatialProcessInfo ProcInfo;
	bool Started = SpatialAutomationCommon::StartSpatialAndPIE(this, ProcInfo, WorkerConfigFile);

	ADD_LATENT_AUTOMATION_COMMAND(FCheckPlayerSpawned(this));

	ADD_LATENT_AUTOMATION_COMMAND(FStopLocalSpatialGame(ProcInfo));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckAuthorityOverGSM, FTestServerAuthoritativeOverGSM*, Test);
bool FCheckAuthorityOverGSM::Update()
{
	UWorld* World = SpatialAutomationCommon::GetActiveGameWorld();
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());

	Worker_EntityId GSMEntityId = SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID;
	Worker_ComponentId PositionComponentId = SpatialConstants::POSITION_COMPONENT_ID;

	bool bAuthoritativeOverGSM = SpatialNetDriver->StaticComponentView->HasAuthority(GSMEntityId, PositionComponentId);

	Test->TestTrue("Server has authority over GSM.", bAuthoritativeOverGSM);
	return true;
}

bool FTestServerAuthoritativeOverGSM::RunTest(const FString& Parameters)
{
	FString WorkerConfigFile = TEXT("default_launch.json");
	SpatialAutomationCommon::SpatialProcessInfo ProcInfo;
	bool Started = SpatialAutomationCommon::StartSpatialAndPIE(this, ProcInfo, WorkerConfigFile);

	ADD_LATENT_AUTOMATION_COMMAND(FCheckAuthorityOverGSM(this));

	ADD_LATENT_AUTOMATION_COMMAND(FStopLocalSpatialGame(ProcInfo));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckCanSpawnAnObject, FTestObjectCanBeSpawned*, Test);
bool FCheckCanSpawnAnObject::Update()
{
	UWorld* World = SpatialAutomationCommon::GetActiveGameWorld();

	FVector Location(0.0f, 0.0f, 0.0f);
	FRotator Rotation(0.0f, 0.0f, 0.0f);
	FActorSpawnParameters SpawnInfo;
	AActor* TestingObject = World->SpawnActor<ATestingObject>(Location, Rotation, SpawnInfo);

	TestingObject->HasAuthority();

	TArray<AActor*> TestingObjects;
	UGameplayStatics::GetAllActorsOfClass(World, ATestingObject::StaticClass(), TestingObjects);

	Test->TestTrue("Can spawn a new test object", TestingObjects.Num() == 1);
	return true;
}

bool FTestObjectCanBeSpawned::RunTest(const FString& Parameters)
{
	FString WorkerConfigFile = TEXT("default_launch.json");
	SpatialAutomationCommon::SpatialProcessInfo ProcInfo;
	bool Started = SpatialAutomationCommon::StartSpatialAndPIE(this, ProcInfo, WorkerConfigFile);

	ADD_LATENT_AUTOMATION_COMMAND(FCheckCanSpawnAnObject(this));

	ADD_LATENT_AUTOMATION_COMMAND(FStopLocalSpatialGame(ProcInfo));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}


#endif // WITH_DEV_AUTOMATION_TESTS
