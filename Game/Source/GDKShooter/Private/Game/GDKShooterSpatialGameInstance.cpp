// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKShooterSpatialGameInstance.h"
#include "Editor.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Game/Components/DeathmatchScoreComponent.h"


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


		// Listen to the callback for HierarchyService component to be added to an entity to get the EntityId of the service to send commands to it.
		ExternalSchema->OnAddComponent([this](const ::improbable::database_sync::DatabaseSyncService::AddComponentOp& Op)
		{
			HierarchyServiceId = Op.EntityId;

			//It can do this by sending the associate_path_with_client command to DBSync in order to allow it.For example, associate_path_with_client(profiles.player1->workerId:Client - {0e61a845 - e978 - 4e5f - b314 - cc6bf1929171}).
			USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
			if (NetDriver == nullptr)
			{
				NetDriver = Cast<USpatialNetDriver>(GetWorldContext()->PendingNetGame->GetNetDriver());
			}
			FString workerId = NetDriver->Connection->GetWorkerId();

			::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::Request Request(TEXT("profiles.UnrealWorker"), workerId);

			ExternalSchema->SendCommandRequest(HierarchyServiceId, Request);
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

		// Listen to updates of items stored in the DB that have been changed outside of the game
		ExternalSchema->OnComponentUpdate([this](const ::improbable::database_sync::DatabaseSyncService::ComponentUpdateOp& Op)
		{
			UDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));
			Deathmatch->ItemUpdateEvent(Op);
		});
		
	});
}
