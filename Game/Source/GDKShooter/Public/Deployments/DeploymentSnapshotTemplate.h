// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Utils/SnapshotGenerationTemplate.h"
#include <WorkerSDK/improbable/c_worker.h>

#include "DeploymentSnapshotTemplate.generated.h"


UCLASS()
class GDKSHOOTER_API UDeploymentSnapshotTemplate : public USnapshotGenerationTemplate
{
	GENERATED_BODY()

public:
	bool WriteToSnapshotOutput(Worker_SnapshotOutputStream* OutputStream, Worker_EntityId& NextEntityId) override;	
};
