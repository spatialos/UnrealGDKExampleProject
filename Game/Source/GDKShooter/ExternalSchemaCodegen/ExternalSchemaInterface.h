#pragma once

#include "CoreMinimal.h"
#include "SpatialConstants.h"
#include "SpatialDispatcher.h"
#include "ExternalSchemaHelperFunctions.h"

#include "improbable/database_sync/DatabaseSyncReference.h"
#include "improbable/database_sync/AssociateDatabaseSync.h"
#include "improbable/database_sync/DatabaseSyncService.h"
#include "improbable/database_sync/HydrationTest.h"

#include <WorkerSDK/improbable/c_worker.h>

class USpatialWorkerConnection;

DECLARE_LOG_CATEGORY_EXTERN(LogExternalSchemaInterface, Log, All);

class ExternalSchemaInterface
{
public:
	ExternalSchemaInterface () = delete;
	ExternalSchemaInterface(USpatialWorkerConnection* Connection, USpatialDispatcher* Dispatcher)
		: SpatialWorkerConnection(Connection), SpatialDispatcher(Dispatcher) {{ }}
	~ExternalSchemaInterface() = default;
	
	void RemoveCallback(USpatialDispatcher::FCallbackId Id);
	
	// Component improbable.database_sync.DatabaseSyncReference = 1906
	void SendComponentUpdate(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncReference::Update& Update);
	USpatialDispatcher::FCallbackId OnAddComponent(const TFunction<void(const ::improbable::database_sync::DatabaseSyncReference::AddComponentOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnRemoveComponent(const TFunction<void(const ::improbable::database_sync::DatabaseSyncReference::RemoveComponentOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnComponentUpdate(const TFunction<void(const ::improbable::database_sync::DatabaseSyncReference::ComponentUpdateOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnAuthorityChange(const TFunction<void(const ::improbable::database_sync::DatabaseSyncReference::AuthorityChangeOp&)>& Callback);
	
	// Component improbable.database_sync.AssociateDatabaseSync = 1907
	void SendComponentUpdate(Worker_EntityId EntityId, const ::improbable::database_sync::AssociateDatabaseSync::Update& Update);
	USpatialDispatcher::FCallbackId OnAddComponent(const TFunction<void(const ::improbable::database_sync::AssociateDatabaseSync::AddComponentOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnRemoveComponent(const TFunction<void(const ::improbable::database_sync::AssociateDatabaseSync::RemoveComponentOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnComponentUpdate(const TFunction<void(const ::improbable::database_sync::AssociateDatabaseSync::ComponentUpdateOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnAuthorityChange(const TFunction<void(const ::improbable::database_sync::AssociateDatabaseSync::AuthorityChangeOp&)>& Callback);
	
	// Component improbable.database_sync.DatabaseSyncService = 1901
	void SendComponentUpdate(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Update& Update);
	USpatialDispatcher::FCallbackId OnAddComponent(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::AddComponentOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnRemoveComponent(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::RemoveComponentOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnComponentUpdate(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::ComponentUpdateOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnAuthorityChange(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::AuthorityChangeOp&)>& Callback);
	
	// command GetItem = 1901
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Request& Request);
	void SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Response& Response);
	USpatialDispatcher::FCallbackId OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::RequestOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::ResponseOp&)>& Callback);
	
	// command GetItems = 1901
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::GetItems::Request& Request);
	void SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::GetItems::Response& Response);
	USpatialDispatcher::FCallbackId OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItems::RequestOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItems::ResponseOp&)>& Callback);
	
	// command Increment = 1901
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request& Request);
	void SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Response& Response);
	USpatialDispatcher::FCallbackId OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::RequestOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::ResponseOp&)>& Callback);
	
	// command Decrement = 1901
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::Decrement::Request& Request);
	void SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::Decrement::Response& Response);
	USpatialDispatcher::FCallbackId OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Decrement::RequestOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Decrement::ResponseOp&)>& Callback);
	
	// command SetParent = 1901
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::SetParent::Request& Request);
	void SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::SetParent::Response& Response);
	USpatialDispatcher::FCallbackId OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::SetParent::RequestOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::SetParent::ResponseOp&)>& Callback);
	
	// command Create = 1901
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::Create::Request& Request);
	void SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::Create::Response& Response);
	USpatialDispatcher::FCallbackId OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Create::RequestOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Create::ResponseOp&)>& Callback);
	
	// command Delete = 1901
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::Delete::Request& Request);
	void SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::Delete::Response& Response);
	USpatialDispatcher::FCallbackId OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Delete::RequestOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Delete::ResponseOp&)>& Callback);
	
	// command Batch = 1901
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::Batch::Request& Request);
	void SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::Batch::Response& Response);
	USpatialDispatcher::FCallbackId OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Batch::RequestOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Batch::ResponseOp&)>& Callback);
	
	// command AssociatePathWithClient = 1901
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::Request& Request);
	void SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::Response& Response);
	USpatialDispatcher::FCallbackId OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::RequestOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::ResponseOp&)>& Callback);
	
	// command GetMetrics = 1901
	Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::Request& Request);
	void SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::Response& Response);
	USpatialDispatcher::FCallbackId OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::RequestOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::ResponseOp&)>& Callback);
	
	// Component improbable.database_sync.HydrationTest = 1903
	void SendComponentUpdate(Worker_EntityId EntityId, const ::improbable::database_sync::HydrationTest::Update& Update);
	USpatialDispatcher::FCallbackId OnAddComponent(const TFunction<void(const ::improbable::database_sync::HydrationTest::AddComponentOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnRemoveComponent(const TFunction<void(const ::improbable::database_sync::HydrationTest::RemoveComponentOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnComponentUpdate(const TFunction<void(const ::improbable::database_sync::HydrationTest::ComponentUpdateOp&)>& Callback);
	USpatialDispatcher::FCallbackId OnAuthorityChange(const TFunction<void(const ::improbable::database_sync::HydrationTest::AuthorityChangeOp&)>& Callback);
	
private:
	void SerializeAndSendComponentUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const ::improbable::SpatialComponentUpdate& Update);
	Worker_RequestId SerializeAndSendCommandRequest(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, const ::improbable::SpatialType& Request);
	void SerializeAndSendCommandResponse(Worker_RequestId RequestId, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, const ::improbable::SpatialType& Response);
	
	USpatialWorkerConnection* SpatialWorkerConnection;
	USpatialDispatcher* SpatialDispatcher;
};
