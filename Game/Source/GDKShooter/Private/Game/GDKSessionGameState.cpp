// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKSessionGameState.h"

#include "TimerManager.h"
#include "UnrealNetwork.h"
#include "SpatialNetDriver.h"
#include "SpatialWorkerConnection.h"
#include "Interop/SpatialStaticComponentView.h"
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
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
	bool AuthoritativeOverSessionEntity = SpatialNetDriver->StaticComponentView->HasAuthority(SessionEntityId, SessionComponentId);

	if (AuthoritativeOverSessionEntity)
	{
		// There's an offset of 1 between the corresponding states of session progress and session state.
		int SessionState = (int)SessionProgress + 1;
		SendStateUpdate(SessionState);
	}
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
	bool AuthoritativeOverSessionProgress = Role == ROLE_Authority;

	if (GetNetMode() != NM_Client && AuthoritativeOverSessionProgress)
	{
		SessionTimer--;

		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
		bool AuthoritativeOverSessionEntity = SpatialNetDriver->StaticComponentView->HasAuthority(SessionEntityId, SessionComponentId);

		if (SessionProgress == EGDKSessionProgress::Lobby && SessionTimer <= 0)
		{
			UE_LOG(LogGDK, Log, TEXT("Advance GameState to Running"));
			SessionProgress = EGDKSessionProgress::Running;
			if (AuthoritativeOverSessionEntity)
			{
				SendStateUpdate(2);
			}
			SessionTimer = GameSessionLength;
		}
		if (SessionProgress == EGDKSessionProgress::Running && SessionTimer <= 0)
		{
			UE_LOG(LogGDK, Log, TEXT("Advance GameState to Results"));
			SessionProgress = EGDKSessionProgress::Results;
			if (AuthoritativeOverSessionEntity)
			{
				SendStateUpdate(3);
			}
			SessionTimer = ResultsSessionLength;
		}
		if (SessionProgress == EGDKSessionProgress::Results && SessionTimer <= 0)
		{
			UE_LOG(LogGDK, Log, TEXT("Advance GameState to Finished"));
			SessionProgress = EGDKSessionProgress::Finished;
			if (AuthoritativeOverSessionEntity)
			{
				SendStateUpdate(4);
			}
		}
	}
}

void AGDKSessionGameState::SendStateUpdate(int NewState)
{
	if (!GetWorld()->GetNetDriver() || !GetWorld()->GetNetDriver()->IsA<USpatialNetDriver>())
	{
		return;
	}

	Worker_EntityId target_entity_id = SessionEntityId;
	Worker_ComponentUpdate component_update = {};
	component_update.component_id = SessionComponentId;
	component_update.schema_type = Schema_CreateComponentUpdate(SessionComponentId);
	Schema_Object* fields_object = Schema_GetComponentUpdateFields(component_update.schema_type);
	Schema_AddInt32(fields_object, 1, NewState);
	Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->SendComponentUpdate(target_entity_id, &component_update);
	UE_LOG(LogGDK, Warning, TEXT("Sending update for session component. State: %i; worker authority: %i"), NewState, Role.GetValue());
}


