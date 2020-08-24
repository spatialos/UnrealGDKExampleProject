// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

// #include "BlastRuntime/Public/BlastMeshActor.h"
// #include "BlastRuntime/Public/TestBlastMesh/TestBlastMeshActor.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGDK, Log, All);

class GDKLogging {
public:
	// Helper method that returns a SpatialOS-specific prefix for log messages, including:
	//   Actor Name
	//   Worker Name
	//   Actor's Corresponding Entity Id
	// Will also work for non-spatial actors, and will report that they don't have an entity id.
	static FString LogPrefix(class AActor* Actor);
};
