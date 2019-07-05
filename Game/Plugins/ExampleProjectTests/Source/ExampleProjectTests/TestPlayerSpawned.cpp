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


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestPlayerControllerGetsCreated, "Spatial.Core.Spawning.PlayerControllerGetsCreated", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ServerContext)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestServerAuthoritativeOverGSM, "Spatial.Core.Spawning.ServerHasAuthorityOverGSM", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ServerContext)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestObjectCanBeSpawnedAndRemoved, "Spatial.Core.Spawning.CanSpawnAndRemoveObjects", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ServerContext)


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
	SpatialAutomationCommon::StartPIE();

	ADD_LATENT_AUTOMATION_COMMAND(FCheckPlayerSpawned(this));

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
	SpatialAutomationCommon::StartPIE();

	ADD_LATENT_AUTOMATION_COMMAND(FCheckAuthorityOverGSM(this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckCanSpawnAnObject, FTestObjectCanBeSpawnedAndRemoved*, Test);
bool FCheckCanSpawnAnObject::Update()
{
	UWorld* World = SpatialAutomationCommon::GetActiveGameWorld();

	TArray<AActor*> TestingObjects;
	UGameplayStatics::GetAllActorsOfClass(World, ATestingObject::StaticClass(), TestingObjects);
	int NumTestingObjectsBeforeSpawn = TestingObjects.Num();

	FVector Location(0.0f, 0.0f, 0.0f);
	FRotator Rotation(0.0f, 0.0f, 0.0f);
	FActorSpawnParameters SpawnInfo;
	AActor* SpawnedObject = World->SpawnActor<ATestingObject>(Location, Rotation, SpawnInfo);

	bool bHasAuthorityOverSpawnedObject = SpawnedObject->HasAuthority();

	UGameplayStatics::GetAllActorsOfClass(World, ATestingObject::StaticClass(), TestingObjects);
	int NumTestingObjectsAfterSpawn = TestingObjects.Num();

	Test->TestTrue("Can spawn a new test object", NumTestingObjectsAfterSpawn == NumTestingObjectsBeforeSpawn + 1);
	Test->TestTrue("Server has authority over the spawned object", bHasAuthorityOverSpawnedObject);

	// Attempt to destroy actor
	bool bActorCouldBeDestroyed = SpawnedObject->Destroy();
	Test->TestTrue("Actor could be destroyed", bActorCouldBeDestroyed);

	UGameplayStatics::GetAllActorsOfClass(World, ATestingObject::StaticClass(), TestingObjects);
	int NumTestingObjectsAfterDestruction = TestingObjects.Num();
	Test->TestTrue("Testing object has been successfully destroyed", NumTestingObjectsAfterDestruction == NumTestingObjectsAfterSpawn - 1);
	
	return true;
}

bool FTestObjectCanBeSpawnedAndRemoved::RunTest(const FString& Parameters)
{
	SpatialAutomationCommon::StartPIE();

	ADD_LATENT_AUTOMATION_COMMAND(FCheckCanSpawnAnObject(this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}


#endif // WITH_DEV_AUTOMATION_TESTS
