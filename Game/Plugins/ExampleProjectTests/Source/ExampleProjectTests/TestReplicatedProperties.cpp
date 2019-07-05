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
#include "Engine/Classes/Kismet/GameplayStatics.h"


#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestDefaultPropertyGetsReplicated, "Spatial.Core.PropertyReplication.DefaultPropertyReplicates", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ServerContext)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestCustomPropertyGetsReplicated, "Spatial.Core.PropertyReplication.CustomPropertyReplicates", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ServerContext)

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckDefaultPropertyGetsReplicated, FTestDefaultPropertyGetsReplicated*, Test);
bool FCheckDefaultPropertyGetsReplicated::Update()
{
	UWorld* World = SpatialAutomationCommon::GetActiveGameWorld();

	ATestLevelScriptActor* TestLevelBlueprint = Cast<ATestLevelScriptActor>(World->GetLevelScriptActor());
	ATestingObject* PlacedTestObject = TestLevelBlueprint->TestObjectReference;

	Test->TestTrue("Initial replicated property value is set to the default value",
		PlacedTestObject->TestProperty.Equals(TestingConstants::DEFAULT_TESTING_PROPERTY_VALUE));

	FString NewPropertyValue = FString(TEXT("Hello world, new property"));
	PlacedTestObject->TestProperty = NewPropertyValue;
	Test->TestTrue("Property value got updated", PlacedTestObject->TestProperty.Equals(NewPropertyValue));

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckCustomPropertyGetsReplicated, FTestCustomPropertyGetsReplicated*, Test);
bool FCheckCustomPropertyGetsReplicated::Update()
{
	UWorld* World = SpatialAutomationCommon::GetActiveGameWorld();

	const FVector TestSpawnLocation(0.0f, 0.0f, 0.0f);
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(TestSpawnLocation);
	ATestingObject* SpawnedObject = World->SpawnActorDeferred<ATestingObject>(ATestingObject::StaticClass(), SpawnTransform);
	if (SpawnedObject != nullptr)
	{
		FString CustomString = FString(TEXT("Some custom test property value!"));
		SpawnedObject->SetTestProperty(CustomString);
		UGameplayStatics::FinishSpawningActor(SpawnedObject, SpawnTransform);
	}

	return true;
}

bool FTestDefaultPropertyGetsReplicated::RunTest(const FString& Parameters)
{
	SpatialAutomationCommon::StartPIE();

	ADD_LATENT_AUTOMATION_COMMAND(FCheckDefaultPropertyGetsReplicated(this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

bool FTestCustomPropertyGetsReplicated::RunTest(const FString& Parameters)
{
	SpatialAutomationCommon::StartPIE();

	ADD_LATENT_AUTOMATION_COMMAND(FCheckCustomPropertyGetsReplicated(this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
