// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKSessionGameState.h"

#include "Interop/SpatialStaticComponentView.h"
#include "TimerManager.h"
#include "UnrealNetwork.h"
#include "SpatialNetDriver.h"
#include "SpatialWorkerConnection.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

void AGDKSessionGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGDKSessionGameState, SessionProgress);
	DOREPLIFETIME(AGDKSessionGameState, SessionTimer);
}

void AGDKSessionGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	if (GetNetMode() != NM_Client)
	{
		if (ConnectedPlayers == 1)
		{
			BeginTimer();
		}
	}
}

void AGDKSessionGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	if (GetNetMode() != NM_Client)
	{
		if (ConnectedPlayers == 0 && SessionProgress == EGDKSessionProgress::Lobby)
		{
			GetWorld()->GetTimerManager().ClearTimer(TickTimer);
		}
	}
}

void AGDKSessionGameState::OnRep_SessionProgress()
{
	SendStateUpdate(SessionProgress);
	TimerEvent.Broadcast(SessionProgress, SessionTimer);
}

void AGDKSessionGameState::OnRep_SessionTimer()
{
	TimerEvent.Broadcast(SessionProgress, SessionTimer);
}

void AGDKSessionGameState::BeginTimer()
{
	if (!GetWorldTimerManager().IsTimerActive(TickTimer))
	{
		SessionProgress = EGDKSessionProgress::Lobby;
		SessionTimer = LobbySessionLength;
		GetWorldTimerManager().SetTimer(TickTimer, this, &AGDKSessionGameState::TickGameTimer, 1.0f, true, 1.0f);
	}
}

void AGDKSessionGameState::TickGameTimer()
{
	const bool bAuthoritativeOverSessionProgress = HasAuthority();

	if (GetNetMode() != NM_Client && bAuthoritativeOverSessionProgress)
	{
		SessionTimer--;

		if (SessionProgress == EGDKSessionProgress::Lobby && SessionTimer <= 0)
		{
			UE_LOG(LogGDK, Log, TEXT("Advance GameState to Running"));
			SessionProgress = EGDKSessionProgress::Running;
			SendStateUpdate(SessionProgress);
			SessionTimer = GameSessionLength;
		}
		if (SessionProgress == EGDKSessionProgress::Running && SessionTimer <= 0)
		{
			UE_LOG(LogGDK, Log, TEXT("Advance GameState to Results"));
			SessionProgress = EGDKSessionProgress::Results;
			SendStateUpdate(SessionProgress);
			SessionTimer = ResultsSessionLength;
		}
		if (SessionProgress == EGDKSessionProgress::Results && SessionTimer <= 0)
		{
			UE_LOG(LogGDK, Log, TEXT("Advance GameState to Finished"));
			SessionProgress = EGDKSessionProgress::Finished;
			SendStateUpdate(SessionProgress);
		}
	}
}

void AGDKSessionGameState::SendStateUpdate(EGDKSessionProgress SessionProgressState)
{
	// Only send the state update if we're using Spatial networking and if we have authority over the session entity.
	if (!GetWorld()->GetNetDriver() || !GetWorld()->GetNetDriver()->IsA<USpatialNetDriver>())
	{
		return;
	}
	else
	{
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
		bool bAuthoritativeOverSessionEntity = SpatialNetDriver->StaticComponentView->HasAuthority(SessionEntityId, SessionComponentId);
		if (!bAuthoritativeOverSessionEntity)
		{
			return;
		}
	}

	// There's an offset of 1 between the corresponding states of session progress and session state.
	int SessionState = static_cast<int>(SessionProgressState) + 1;

	Worker_EntityId target_entity_id = SessionEntityId;
	Worker_ComponentUpdate component_update = {};
	component_update.component_id = SessionComponentId;
	component_update.schema_type = Schema_CreateComponentUpdate(SessionComponentId);
	Schema_Object* fields_object = Schema_GetComponentUpdateFields(component_update.schema_type);
	Schema_AddInt32(fields_object, 1, SessionState);
	Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->SendComponentUpdate(target_entity_id, &component_update);
}


