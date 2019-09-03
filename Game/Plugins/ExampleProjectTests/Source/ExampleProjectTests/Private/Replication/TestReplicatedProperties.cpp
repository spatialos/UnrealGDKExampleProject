//#pragma once
//#include "AutomationCommon.h"
//#include "Editor/UnrealEd/Public/Tests/AutomationEditorCommon.h"
//#include "Kismet/GameplayStatics.h"
//#include "CoreMinimal.h"
//#include "Controllers/GDKPlayerController.h"
//#include "SpatialAutomationCommon.h"
//#include "SpatialNetDriver.h"
//#include "GDKCharacter.h"
//#include "Interop/SpatialStaticComponentView.h"
//#include "TestingObject.h"
//#include "TestLevelScriptActor.h"
//#include "TestingConstants.h"
//#include "Engine/Classes/Kismet/GameplayStatics.h"
//#include "SpatialAutomationTest.h"
//
//
//#if WITH_DEV_AUTOMATION_TESTS
//
//LATENT_TEST_COMMAND_ONE_PARAMETER(FCheckDefaultPropertyGetsReplicated, Test)
//{
//	UWorld* World = SpatialAutomationCommon::GetActiveGameWorld();
//
//	ATestLevelScriptActor* TestLevelBlueprint = Cast<ATestLevelScriptActor>(World->GetLevelScriptActor());
//	ATestingObject* PlacedTestObject = TestLevelBlueprint->TestObjectReference;
//
//	Test->TestTrue("Initial replicated property value is set to the default value",
//		PlacedTestObject->TestProperty.Equals(TestingConstants::DEFAULT_TESTING_PROPERTY_VALUE));
//
//	return true;
//}
//
//DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckOnRepGetsCalledOnClient, FTestOnRepGetsCalledOnClient*, Test);
//bool FCheckOnRepGetsCalledOnClient::Update()
//{
//	UWorld* World = SpatialAutomationCommon::GetActiveGameWorld();
//
//	ATestLevelScriptActor* TestLevelBlueprint = Cast<ATestLevelScriptActor>(World->GetLevelScriptActor());
//	ATestingObject* PlacedTestObject = TestLevelBlueprint->TestObjectReference;
//
//	int NumTestPropertyReplicationsBefore = PlacedTestObject->NumTestPropertyUpdatesReceived;
//
//	FString NewPropertyValue = FString(TEXT("Hello world, new property"));
//	PlacedTestObject->TestProperty = NewPropertyValue;
//
//	int NumTestPropertyReplicationsAfter = PlacedTestObject->NumTestPropertyUpdatesReceived;
//
//	Test->TestTrue("Property value got updated", PlacedTestObject->TestProperty.Equals(NewPropertyValue));
//	// TODO timing, do we need to wait for a bit here?
//	Test->TestTrue("OnRep was called on client when test property value got updated.", NumTestPropertyReplicationsAfter == NumTestPropertyReplicationsBefore + 1);
//
//	return true;
//}
//
//DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckCustomPropertyGetsReplicated, FTestCustomPropertyGetsReplicated*, Test);
//bool FCheckCustomPropertyGetsReplicated::Update()
//{
//	UWorld* World = SpatialAutomationCommon::GetActiveGameWorld();
//
//	FString CustomString = FString(TEXT("Some custom test property value!"));
//
//	const FVector TestSpawnLocation(0.0f, 0.0f, 0.0f);
//	FTransform SpawnTransform;
//	SpawnTransform.SetLocation(TestSpawnLocation);
//	ATestingObject* SpawnedObject = World->SpawnActorDeferred<ATestingObject>(ATestingObject::StaticClass(), SpawnTransform);
//	if (SpawnedObject != nullptr)
//	{
//		SpawnedObject->SetTestProperty(CustomString);
//		UGameplayStatics::FinishSpawningActor(SpawnedObject, SpawnTransform);
//	}
//
//	Test->TestTrue("Spawning with a custom property correctly replicates this value.", SpawnedObject->TestProperty.Equals(CustomString));
//	return true;
//}
//
//SPATIAL_TEST(DefaultPropertyGetsReplicated, Replication)
//{
//	SpatialAutomationCommon::StartPIE();
//
//	ADD_LATENT_AUTOMATION_COMMAND(FCheckDefaultPropertyGetsReplicated(this));
//
//	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
//	return true;
//}
//
//SPATIAL_TEST(CustomPropertyGetsReplicated, Replication)
//{
//	SpatialAutomationCommon::StartPIE();
//
//	ADD_LATENT_AUTOMATION_COMMAND(FCheckCustomPropertyGetsReplicated(this));
//
//	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
//	return true;
//}
//
//
//SPATIAL_TEST(OnRepGetsCalledOnClient, Replication)
//{
//	SpatialAutomationCommon::StartPIE();
//
//	ADD_LATENT_AUTOMATION_COMMAND(FCheckOnRepGetsCalledOnClient(this));
//
//	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
//	return true;
//}
//
//#endif // WITH_DEV_AUTOMATION_TESTS
