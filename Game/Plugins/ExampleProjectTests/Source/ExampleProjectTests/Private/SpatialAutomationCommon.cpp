#include "SpatialAutomationCommon.h"

#include "Engine.h"
#include "EngineUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

const float SpatialAutomationCommon::kDefaultPlayerJoinDelay = 20.0f;

/**
 * Retrieves the active world for the game (i.e the world for the game currently being played).
 */
UWorld* SpatialAutomationCommon::GetActiveGameWorld()
{
	UWorld* TestWorld = nullptr;
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts)
	{
		if (((Context.WorldType == EWorldType::PIE) || (Context.WorldType == EWorldType::Game)) && (Context.World() != NULL))
		{
			TestWorld = Context.World();
			break;
		}
	}

	return TestWorld;
}

int SpatialAutomationCommon::GetNumberOfActorsOfClass(TSubclassOf<AActor> ActorClass)
{
	UWorld* World = GetActiveGameWorld();
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(World, ActorClass, Actors);

	return Actors.Num();
}

bool TestIfAtLeastGivenNumberOfActorsExist::Update()
{
	int NumActorsSpawnedInTheWorld = SpatialAutomationCommon::GetNumberOfActorsOfClass(ActorClass);

	FString TestMessage = FString::Printf(TEXT("Expected at least %d actors of class %s to be present in the world."), LowerLimit, *ActorClass->GetFName().ToString());
	Test->TestTrue(TestMessage, NumActorsSpawnedInTheWorld >= 1);
	return true;
}

void SpatialAutomationCommon::StartPIE(float PlayerJoinDelay)
{
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(PlayerJoinDelay));
}

#endif // WITH_DEV_AUTOMATION_TESTS
