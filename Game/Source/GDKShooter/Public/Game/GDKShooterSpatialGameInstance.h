// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "GDKShooter/ExternalSchemaCodegen/ExternalSchemaInterface.h"
#include "GDKShooterSpatialGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class GDKSHOOTER_API UGDKShooterSpatialGameInstance : public USpatialGameInstance
{
	GENERATED_BODY()
	
public:

	void Init();

	ExternalSchemaInterface* ExternalSchema;

	Worker_EntityId HierarchyServiceId;

};
