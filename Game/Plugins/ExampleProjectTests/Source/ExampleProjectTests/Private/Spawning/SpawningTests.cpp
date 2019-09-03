#pragma once

#include "AutomationCommon.h"
#include "SpatialAutomationCommon.h"
#include "Spawning/SpawningCommon.h"
#include "Testing/NonReplicatedTestingObject.h"
#include "Testing/TestingObject.h"

#if WITH_DEV_AUTOMATION_TESTS

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FClientHasCheckedOutSpawnedActor, ATestingObject*, SpawnedObject, FAutomationTestBase*, Test);
bool FClientHasCheckedOutSpawnedActor::Update()
{
	Test->TestTrue("Client has checked out the replicated object spawned by server.", SpawnedObject->HasBeenCheckedOutByClient);

	return true;
}

LATENT_TEST_COMMAND_ONE_PARAMETER(FSpawnReplicatedObject, Test)
{
	AActor* SpawnedObject = SpawningCommon::SpawnActorAtOrigin<ATestingObject>();

	Test->TestTrue("Can spawn a new test object", SpawnedObject != nullptr);
	Test->TestTrue("Server has authority over the spawned object", SpawnedObject->HasAuthority());

	ATestingObject* NewTestingObject = Cast<ATestingObject>(SpawnedObject);
	Test->TestNotNull("Spawned object is of the correct class (ATestingObject)", NewTestingObject);

	ADD_LATENT_AUTOMATION_COMMAND_WITH_DELAY(FClientHasCheckedOutSpawnedActor(NewTestingObject, Test), 2.f);

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());

	return true; 
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FClientHasNotCheckedOutSpawnedActor, ATestingObject*, SpawnedObject, FAutomationTestBase*, Test);
bool FClientHasNotCheckedOutSpawnedActor::Update()
{
	Test->TestFalse("Client has not checked out the non-replicated object spawned by server.", SpawnedObject->HasBeenCheckedOutByClient);

	return true;
}

LATENT_TEST_COMMAND_ONE_PARAMETER(FSpawnNonReplicatedObject, Test)
{
	AActor* SpawnedObject = SpawningCommon::SpawnActorAtOrigin<ANonReplicatedTestingObject>();

	Test->TestTrue("Can spawn a new test object", SpawnedObject != nullptr);
	Test->TestTrue("Server has authority over the spawned object", SpawnedObject->HasAuthority());

	ANonReplicatedTestingObject* NewTestingObject = Cast<ANonReplicatedTestingObject>(SpawnedObject);
	Test->TestNotNull("Spawned object is of the correct class (ANonReplicatedTestingObject)", NewTestingObject);

	ADD_LATENT_AUTOMATION_COMMAND_WITH_DELAY(FClientHasNotCheckedOutSpawnedActor(NewTestingObject, Test), 2.f);

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());

	return true;
}

SPATIAL_TEST(ReplicatedObjectSpawnsCorrectly, Spawning)
{
	SpatialAutomationCommon::StartPIE();

	ADD_LATENT_AUTOMATION_COMMAND(FSpawnReplicatedObject(this));

	return true;
}

SPATIAL_TEST(NonReplicatedObjectSpawnsCorrectly, Spawning)
{
	SpatialAutomationCommon::StartPIE();

	ADD_LATENT_AUTOMATION_COMMAND(FSpawnNonReplicatedObject(this));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
