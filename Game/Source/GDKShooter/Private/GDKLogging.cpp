// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKLogging.h"

#include "CoreMinimal.h"
#include "Engine/NetDriver.h"
#include "GameFramework/Actor.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"


DEFINE_LOG_CATEGORY(LogGDK);

FString GDKLogging::LogPrefix(AActor* Actor)
{
	FString WorkerId("UNKNOWN");
	int32 EntityId = -1;
	if (USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(Actor->GetNetDriver()))
	{
		WorkerId = SpatialNetDriver->Connection->GetWorkerId();
		EntityId = SpatialNetDriver->PackageMap->GetEntityIdFromObject(Actor);
	}
	else
	{
		// No SpatialOS net driver, so just return the actor's name.
		return Actor->GetName();
	}

	if (EntityId == 0)
	{
		return FString::Printf(TEXT("%s %s (non-spatial actor)"), *WorkerId, *Actor->GetName());
	}

	return FString::Printf(TEXT("%s %s (%d)"), *WorkerId, *Actor->GetName(), EntityId);
}
