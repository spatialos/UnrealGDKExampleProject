// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Deployments/DeploymentSnapshotTemplate.h"

#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "Schema/StandardLibrary.h"

bool UDeploymentSnapshotTemplate::WriteToSnapshotOutput(Worker_SnapshotOutputStream* OutputStream, Worker_EntityId& NextEntityId)
{
	Worker_Entity SessionEntity;
	SessionEntity.entity_id = NextEntityId;

	TArray<Worker_ComponentData> Components;

	const WorkerAttributeSet DeploymentManagerAttributeSet{ TArray<FString>{TEXT("DeploymentManager")} };
	const WorkerRequirementSet DeploymentManagerPermission{ DeploymentManagerAttributeSet };
	const WorkerRequirementSet AnyWorkerPermission{ {SpatialConstants::UnrealClientAttributeSet, SpatialConstants::UnrealServerAttributeSet, DeploymentManagerAttributeSet } };

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(1000, SpatialConstants::UnrealServerPermission);
	ComponentWriteAcl.Add(1001, DeploymentManagerPermission);

	// Serialize Session component data
	Worker_ComponentData SessionComponentData{};
	SessionComponentData.component_id = 1000;
	SessionComponentData.schema_type = Schema_CreateComponentData();
	Schema_Object* SessionComponentDataObject = Schema_GetComponentDataFields(SessionComponentData.schema_type);
	Schema_AddInt32(SessionComponentDataObject, 1, 1);

	Worker_ComponentData DeploymentComponentData{};
	DeploymentComponentData.component_id = 1001;
	DeploymentComponentData.schema_type = Schema_CreateComponentData();

	Components.Add(SpatialGDK::Position(SpatialGDK::DeploymentOrigin).CreatePositionData());
	Components.Add(SpatialGDK::Metadata(TEXT("Session")).CreateMetadataData());
	Components.Add(SpatialGDK::Persistence().CreatePersistenceData());
	Components.Add(SpatialGDK::EntityAcl(AnyWorkerPermission, ComponentWriteAcl).CreateEntityAclData());
	Components.Add(SessionComponentData);
	Components.Add(DeploymentComponentData);

	SessionEntity.component_count = Components.Num();
	SessionEntity.components = Components.GetData();

	Worker_SnapshotOutputStream_WriteEntity(OutputStream, &SessionEntity);
	bool success = Worker_SnapshotOutputStream_GetState(OutputStream).stream_state == WORKER_STREAM_STATE_GOOD;
	if (success) {
		NextEntityId++;
	}

	return success;
}


