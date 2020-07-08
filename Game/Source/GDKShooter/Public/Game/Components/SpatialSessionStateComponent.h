// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialSessionStateComponent.generated.h"

// States of a session, to match the schema of the deployment manager
UENUM(BlueprintType)
enum class EGDKSessionProgress : uint8
{
	Lobby				UMETA(DisplayName = "Lobby"),
	Running				UMETA(DisplayName = "Running"),
	Results				UMETA(DisplayName = "Results"),
	Finished			UMETA(DisplayName = "Finished"),
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API USpatialSessionStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SendStateUpdate(EGDKSessionProgress SessionProgressState);
	
protected:
	Worker_EntityId SessionEntityId = 39;
	Worker_ComponentId SessionComponentId = 1000;
};
