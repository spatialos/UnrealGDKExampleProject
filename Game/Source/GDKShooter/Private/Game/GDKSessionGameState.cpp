// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKSessionGameState.h"

#include "TimerManager.h"
#include "UnrealNetwork.h"
#include "SpatialNetDriver.h"
#include "SpatialWorkerConnection.h"
#include "c_worker.h"
#include "c_schema.h"


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
	if (GetNetMode() != NM_Client)
	{
		SessionTimer--;

		if (SessionProgress == EGDKSessionProgress::Lobby && SessionTimer <= 0)
		{
			UE_LOG(LogGDK, Log, TEXT("Advance GameState to Running"));
			SessionProgress = EGDKSessionProgress::Running;
			SendStateUpdate(2);
			SessionTimer = GameSessionLength;
		}
		if (SessionProgress == EGDKSessionProgress::Running && SessionTimer <= 0)
		{
			UE_LOG(LogGDK, Log, TEXT("Advance GameState to Results"));
			SessionProgress = EGDKSessionProgress::Results;
			SendStateUpdate(3);
			SessionTimer = ResultsSessionLength;
		}
		if (SessionProgress == EGDKSessionProgress::Results && SessionTimer <= 0)
		{
			UE_LOG(LogGDK, Log, TEXT("Advance GameState to Finished"));
			SessionProgress = EGDKSessionProgress::Finished;
			SendStateUpdate(4);
		}
	}
}

void AGDKSessionGameState::SendStateUpdate(int NewState)
{
	if (!GetWorld()->GetNetDriver() || !GetWorld()->GetNetDriver()->IsA<USpatialNetDriver>())
	{
		return;
	}

	Worker_EntityId target_entity_id = 39;
	Worker_ComponentUpdate component_update;
	component_update.component_id = 1000;
	component_update.schema_type = Schema_CreateComponentUpdate(1000);
	Schema_Object* fields_object = Schema_GetComponentUpdateFields(component_update.schema_type);
	Schema_AddInt32(fields_object, 1, NewState);
	Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->SendComponentUpdate(target_entity_id, &component_update);
}


