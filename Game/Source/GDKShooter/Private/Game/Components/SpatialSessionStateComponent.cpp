// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Game/Components/SpatialSessionStateComponent.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Interop/Connection/SpatialWorkerConnection.h"

void USpatialSessionStateComponent::SendStateUpdate(EGDKSessionProgress SessionProgressState)
{
	// Only send the state update if we're using Spatial networking and if we have authority over the session entity.
	UNetDriver* NetDriver = GetWorld()->GetNetDriver();
	if (NetDriver == nullptr || !NetDriver->IsA<USpatialNetDriver>())
	{
		return;
	}

	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(NetDriver);
	bool bAuthoritativeOverSessionEntity = SpatialNetDriver->StaticComponentView->HasAuthority(SessionEntityId, SessionComponentId);
	if (!bAuthoritativeOverSessionEntity)
	{
		return;
	}

	// There's an offset of 1 between the corresponding states of session progress and session state.
	int SessionState = static_cast<int>(SessionProgressState) + 1;

	Worker_EntityId target_entity_id = SessionEntityId;
	FWorkerComponentUpdate component_update = {};
	component_update.component_id = SessionComponentId;
	component_update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* fields_object = Schema_GetComponentUpdateFields(component_update.schema_type);
	Schema_AddInt32(fields_object, 1, SessionState);
	SpatialNetDriver->Connection->SendComponentUpdate(target_entity_id, &component_update);
}
