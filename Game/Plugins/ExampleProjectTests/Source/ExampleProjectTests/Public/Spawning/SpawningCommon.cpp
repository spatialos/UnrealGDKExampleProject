#include "Spawning/SpawningCommon.h"

#include "SpatialAutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

#endif // WITH_DEV_AUTOMATION_TESTS

template< class T >
AActor* SpawningCommon::SpawnActorAtOrigin()
{
	UWorld* World = SpatialAutomationCommon::GetActiveGameWorld();
	AActor* SpawnedObject = World->SpawnActor<T>();

	return SpawnedObject;
}
