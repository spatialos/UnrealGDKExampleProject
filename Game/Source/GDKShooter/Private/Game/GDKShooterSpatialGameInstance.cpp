// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKShooterSpatialGameInstance.h"
#include "Editor.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Game/Components/DeathmatchScoreComponent.h"

#include "ExternalSchemaCodegen/improbable/database_sync/DatabaseSyncReference.h"
#include "ExternalSchemaCodegen/improbable/database_sync/AssociateDatabaseSync.h"
#include "ExternalSchemaCodegen/improbable/database_sync/DatabaseSyncService.h"
//#include "ExternalSchemaCodegen/improbable/database_sync/Profile.h"


void UGDKShooterSpatialGameInstance::Init()
{
	OnConnected.AddLambda([this]() {
		// On the client the world may not be completely set up, if so we can use the PendingNetGame
		USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
		if (NetDriver == nullptr)
		{
			NetDriver = Cast<USpatialNetDriver>(GetWorldContext()->PendingNetGame->GetNetDriver());
		}

		ExternalSchema = new ExternalSchemaInterface(NetDriver->Connection, NetDriver->Dispatcher);

		//UDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));


		// Listen to the callback for HierarchyService component to be added to an entity to get the EntityId of the service to send commands to it.
		ExternalSchema->OnAddComponent([this](const ::improbable::database_sync::DatabaseSyncService::AddComponentOp& Op)
		{
			HierarchyServiceId = Op.EntityId;
		});


		// Listen to updates of items stored in the DB that have been changed outside of the game
		ExternalSchema->OnComponentUpdate([this](const ::improbable::database_sync::DatabaseSyncService::ComponentUpdateOp& Op)
		{
			UDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));
			Deathmatch->ItemUpdateEvent(Op);
		});


		// Listen to callbacks for using the DB, like seaching for items, creating them and increasing the value
		ExternalSchema->OnCommandResponse([this](const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::ResponseOp& Op)
		{
			UDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));
			Deathmatch->GetItemResponse(Op);
		});

		ExternalSchema->OnCommandResponse([this](const ::improbable::database_sync::DatabaseSyncService::Commands::Create::ResponseOp& Op)
		{
			UDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));
			Deathmatch->CreateItemResponse(Op);
		});
		
		ExternalSchema->OnCommandResponse([this] (const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::ResponseOp& Op)
		{
			UDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));
			Deathmatch->IncrementResponse(Op);
		});
		
	});
}
