#include "ExternalSchemaInterface.h"
#include "Connection/SpatialWorkerConnection.h"

DEFINE_LOG_CATEGORY(LogExternalSchemaInterface);

void ExternalSchemaInterface::RemoveCallback(USpatialDispatcher::FCallbackId Id)
{
	SpatialDispatcher->RemoveOpCallback(Id);
}

void ExternalSchemaInterface::SerializeAndSendComponentUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const ::improbable::SpatialComponentUpdate& Update)
{
	Worker_ComponentUpdate SerializedUpdate = {};
	SerializedUpdate.component_id = ComponentId;
	SerializedUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
	Update.Serialize(SerializedUpdate.schema_type);
	SpatialWorkerConnection->SendComponentUpdate(EntityId, &SerializedUpdate);
}

Worker_RequestId ExternalSchemaInterface::SerializeAndSendCommandRequest(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, const ::improbable::SpatialType& Request)
{
	Worker_CommandRequest SerializedRequest = {};
	SerializedRequest.component_id = ComponentId;
	SerializedRequest.schema_type = Schema_CreateCommandRequest(ComponentId, CommandIndex);
	Schema_Object* RequestObject = Schema_GetCommandRequestObject(SerializedRequest.schema_type);
	Request.Serialize(RequestObject);
	return SpatialWorkerConnection->SendCommandRequest(EntityId, &SerializedRequest, CommandIndex);
}

void ExternalSchemaInterface::SerializeAndSendCommandResponse(Worker_RequestId RequestId, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, const ::improbable::SpatialType& Response)
{
	Worker_CommandResponse SerializedResponse = {};
	SerializedResponse.component_id = ComponentId;
	SerializedResponse.schema_type = Schema_CreateCommandResponse(ComponentId, CommandIndex);
	Schema_Object* ResponseObject = Schema_GetCommandResponseObject(SerializedResponse.schema_type);
	Response.Serialize(ResponseObject);
	return SpatialWorkerConnection->SendCommandResponse(RequestId, &SerializedResponse);
}

// Component improbable.database_sync.DatabaseSyncReference = 1906
void ExternalSchemaInterface::SendComponentUpdate(const Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncReference::Update& Update)
{             
	SerializeAndSendComponentUpdate(EntityId, 1906, Update);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnAddComponent(const TFunction<void(const ::improbable::database_sync::DatabaseSyncReference::AddComponentOp&)>& Callback)
{
	return SpatialDispatcher->OnAddComponent(1906, [Callback](const Worker_AddComponentOp& Op)
	{
		::improbable::database_sync::DatabaseSyncReference Data = ::improbable::database_sync::DatabaseSyncReference::Deserialize(Op.data.schema_type);
		Callback(::improbable::database_sync::DatabaseSyncReference::AddComponentOp(Op.entity_id, Op.data.component_id, Data));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnRemoveComponent(const TFunction<void(const ::improbable::database_sync::DatabaseSyncReference::RemoveComponentOp&)>& Callback)
{
	return SpatialDispatcher->OnRemoveComponent(1906, [Callback](const Worker_RemoveComponentOp& Op)
	{
		Callback(::improbable::database_sync::DatabaseSyncReference::RemoveComponentOp(Op.entity_id, Op.component_id));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnComponentUpdate(const TFunction<void(const ::improbable::database_sync::DatabaseSyncReference::ComponentUpdateOp&)>& Callback)
{
	return SpatialDispatcher->OnComponentUpdate(1906, [Callback](const Worker_ComponentUpdateOp& Op)
	{
		::improbable::database_sync::DatabaseSyncReference::Update Update = ::improbable::database_sync::DatabaseSyncReference::Update::Deserialize(Op.update.schema_type);
		Callback(::improbable::database_sync::DatabaseSyncReference::ComponentUpdateOp(Op.entity_id, Op.update.component_id, Update));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnAuthorityChange(const TFunction<void(const ::improbable::database_sync::DatabaseSyncReference::AuthorityChangeOp&)>& Callback)
{
	return SpatialDispatcher->OnAuthorityChange(1906, [Callback](const Worker_AuthorityChangeOp& Op)
	{
		Callback(::improbable::database_sync::DatabaseSyncReference::AuthorityChangeOp(Op.entity_id, Op.component_id, static_cast<Worker_Authority>(Op.authority)));
	});
}


// Component improbable.database_sync.AssociateDatabaseSync = 1907
void ExternalSchemaInterface::SendComponentUpdate(const Worker_EntityId EntityId, const ::improbable::database_sync::AssociateDatabaseSync::Update& Update)
{             
	SerializeAndSendComponentUpdate(EntityId, 1907, Update);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnAddComponent(const TFunction<void(const ::improbable::database_sync::AssociateDatabaseSync::AddComponentOp&)>& Callback)
{
	return SpatialDispatcher->OnAddComponent(1907, [Callback](const Worker_AddComponentOp& Op)
	{
		::improbable::database_sync::AssociateDatabaseSync Data = ::improbable::database_sync::AssociateDatabaseSync::Deserialize(Op.data.schema_type);
		Callback(::improbable::database_sync::AssociateDatabaseSync::AddComponentOp(Op.entity_id, Op.data.component_id, Data));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnRemoveComponent(const TFunction<void(const ::improbable::database_sync::AssociateDatabaseSync::RemoveComponentOp&)>& Callback)
{
	return SpatialDispatcher->OnRemoveComponent(1907, [Callback](const Worker_RemoveComponentOp& Op)
	{
		Callback(::improbable::database_sync::AssociateDatabaseSync::RemoveComponentOp(Op.entity_id, Op.component_id));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnComponentUpdate(const TFunction<void(const ::improbable::database_sync::AssociateDatabaseSync::ComponentUpdateOp&)>& Callback)
{
	return SpatialDispatcher->OnComponentUpdate(1907, [Callback](const Worker_ComponentUpdateOp& Op)
	{
		::improbable::database_sync::AssociateDatabaseSync::Update Update = ::improbable::database_sync::AssociateDatabaseSync::Update::Deserialize(Op.update.schema_type);
		Callback(::improbable::database_sync::AssociateDatabaseSync::ComponentUpdateOp(Op.entity_id, Op.update.component_id, Update));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnAuthorityChange(const TFunction<void(const ::improbable::database_sync::AssociateDatabaseSync::AuthorityChangeOp&)>& Callback)
{
	return SpatialDispatcher->OnAuthorityChange(1907, [Callback](const Worker_AuthorityChangeOp& Op)
	{
		Callback(::improbable::database_sync::AssociateDatabaseSync::AuthorityChangeOp(Op.entity_id, Op.component_id, static_cast<Worker_Authority>(Op.authority)));
	});
}


// Component improbable.database_sync.DatabaseSyncService = 1901
void ExternalSchemaInterface::SendComponentUpdate(const Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Update& Update)
{             
	SerializeAndSendComponentUpdate(EntityId, 1901, Update);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnAddComponent(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::AddComponentOp&)>& Callback)
{
	return SpatialDispatcher->OnAddComponent(1901, [Callback](const Worker_AddComponentOp& Op)
	{
		::improbable::database_sync::DatabaseSyncService Data = ::improbable::database_sync::DatabaseSyncService::Deserialize(Op.data.schema_type);
		Callback(::improbable::database_sync::DatabaseSyncService::AddComponentOp(Op.entity_id, Op.data.component_id, Data));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnRemoveComponent(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::RemoveComponentOp&)>& Callback)
{
	return SpatialDispatcher->OnRemoveComponent(1901, [Callback](const Worker_RemoveComponentOp& Op)
	{
		Callback(::improbable::database_sync::DatabaseSyncService::RemoveComponentOp(Op.entity_id, Op.component_id));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnComponentUpdate(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::ComponentUpdateOp&)>& Callback)
{
	return SpatialDispatcher->OnComponentUpdate(1901, [Callback](const Worker_ComponentUpdateOp& Op)
	{
		::improbable::database_sync::DatabaseSyncService::Update Update = ::improbable::database_sync::DatabaseSyncService::Update::Deserialize(Op.update.schema_type);
		Callback(::improbable::database_sync::DatabaseSyncService::ComponentUpdateOp(Op.entity_id, Op.update.component_id, Update));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnAuthorityChange(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::AuthorityChangeOp&)>& Callback)
{
	return SpatialDispatcher->OnAuthorityChange(1901, [Callback](const Worker_AuthorityChangeOp& Op)
	{
		Callback(::improbable::database_sync::DatabaseSyncService::AuthorityChangeOp(Op.entity_id, Op.component_id, static_cast<Worker_Authority>(Op.authority)));
	});
}

Worker_RequestId ExternalSchemaInterface::SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Request& Request)
{
	return SerializeAndSendCommandRequest(EntityId, 1901, 1, Request.Data);
}

void ExternalSchemaInterface::SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Response& Response)
{
	SerializeAndSendCommandResponse(RequestId, 1901, 1, Response.Data);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::RequestOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandRequest(1901, [Callback](const Worker_CommandRequestOp& Op)
	{
		auto Request = ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Request(::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Request::Type::Deserialize(Schema_GetCommandRequestObject(Op.request.schema_type)));
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::GetItem::RequestOp(Op.entity_id, Op.request_id, Op.timeout_millis, Op.caller_worker_id, Op.caller_attribute_set, Request));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::ResponseOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandResponse(1901, [Callback](const Worker_CommandResponseOp& Op)
	{
		auto Response = Op.status_code == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS  ? ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Response(::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Response::Type::Deserialize(Schema_GetCommandResponseObject(Op.response.schema_type))) : ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Response();
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::GetItem::ResponseOp(Op.entity_id, Op.request_id, Op.status_code, Op.message, Op.command_id, Response));
	});
}

Worker_RequestId ExternalSchemaInterface::SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::GetItems::Request& Request)
{
	return SerializeAndSendCommandRequest(EntityId, 1901, 2, Request.Data);
}

void ExternalSchemaInterface::SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::GetItems::Response& Response)
{
	SerializeAndSendCommandResponse(RequestId, 1901, 2, Response.Data);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItems::RequestOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandRequest(1901, [Callback](const Worker_CommandRequestOp& Op)
	{
		auto Request = ::improbable::database_sync::DatabaseSyncService::Commands::GetItems::Request(::improbable::database_sync::DatabaseSyncService::Commands::GetItems::Request::Type::Deserialize(Schema_GetCommandRequestObject(Op.request.schema_type)));
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::GetItems::RequestOp(Op.entity_id, Op.request_id, Op.timeout_millis, Op.caller_worker_id, Op.caller_attribute_set, Request));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItems::ResponseOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandResponse(1901, [Callback](const Worker_CommandResponseOp& Op)
	{
		auto Response = Op.status_code == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS  ? ::improbable::database_sync::DatabaseSyncService::Commands::GetItems::Response(::improbable::database_sync::DatabaseSyncService::Commands::GetItems::Response::Type::Deserialize(Schema_GetCommandResponseObject(Op.response.schema_type))) : ::improbable::database_sync::DatabaseSyncService::Commands::GetItems::Response();
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::GetItems::ResponseOp(Op.entity_id, Op.request_id, Op.status_code, Op.message, Op.command_id, Response));
	});
}

Worker_RequestId ExternalSchemaInterface::SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request& Request)
{
	return SerializeAndSendCommandRequest(EntityId, 1901, 3, Request.Data);
}

void ExternalSchemaInterface::SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Response& Response)
{
	SerializeAndSendCommandResponse(RequestId, 1901, 3, Response.Data);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::RequestOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandRequest(1901, [Callback](const Worker_CommandRequestOp& Op)
	{
		auto Request = ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request(::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request::Type::Deserialize(Schema_GetCommandRequestObject(Op.request.schema_type)));
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::Increment::RequestOp(Op.entity_id, Op.request_id, Op.timeout_millis, Op.caller_worker_id, Op.caller_attribute_set, Request));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::ResponseOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandResponse(1901, [Callback](const Worker_CommandResponseOp& Op)
	{
		auto Response = Op.status_code == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS  ? ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Response(::improbable::database_sync::DatabaseSyncService::Commands::Increment::Response::Type::Deserialize(Schema_GetCommandResponseObject(Op.response.schema_type))) : ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Response();
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::Increment::ResponseOp(Op.entity_id, Op.request_id, Op.status_code, Op.message, Op.command_id, Response));
	});
}

Worker_RequestId ExternalSchemaInterface::SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::Decrement::Request& Request)
{
	return SerializeAndSendCommandRequest(EntityId, 1901, 4, Request.Data);
}

void ExternalSchemaInterface::SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::Decrement::Response& Response)
{
	SerializeAndSendCommandResponse(RequestId, 1901, 4, Response.Data);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Decrement::RequestOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandRequest(1901, [Callback](const Worker_CommandRequestOp& Op)
	{
		auto Request = ::improbable::database_sync::DatabaseSyncService::Commands::Decrement::Request(::improbable::database_sync::DatabaseSyncService::Commands::Decrement::Request::Type::Deserialize(Schema_GetCommandRequestObject(Op.request.schema_type)));
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::Decrement::RequestOp(Op.entity_id, Op.request_id, Op.timeout_millis, Op.caller_worker_id, Op.caller_attribute_set, Request));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Decrement::ResponseOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandResponse(1901, [Callback](const Worker_CommandResponseOp& Op)
	{
		auto Response = Op.status_code == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS  ? ::improbable::database_sync::DatabaseSyncService::Commands::Decrement::Response(::improbable::database_sync::DatabaseSyncService::Commands::Decrement::Response::Type::Deserialize(Schema_GetCommandResponseObject(Op.response.schema_type))) : ::improbable::database_sync::DatabaseSyncService::Commands::Decrement::Response();
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::Decrement::ResponseOp(Op.entity_id, Op.request_id, Op.status_code, Op.message, Op.command_id, Response));
	});
}

Worker_RequestId ExternalSchemaInterface::SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::SetParent::Request& Request)
{
	return SerializeAndSendCommandRequest(EntityId, 1901, 5, Request.Data);
}

void ExternalSchemaInterface::SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::SetParent::Response& Response)
{
	SerializeAndSendCommandResponse(RequestId, 1901, 5, Response.Data);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::SetParent::RequestOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandRequest(1901, [Callback](const Worker_CommandRequestOp& Op)
	{
		auto Request = ::improbable::database_sync::DatabaseSyncService::Commands::SetParent::Request(::improbable::database_sync::DatabaseSyncService::Commands::SetParent::Request::Type::Deserialize(Schema_GetCommandRequestObject(Op.request.schema_type)));
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::SetParent::RequestOp(Op.entity_id, Op.request_id, Op.timeout_millis, Op.caller_worker_id, Op.caller_attribute_set, Request));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::SetParent::ResponseOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandResponse(1901, [Callback](const Worker_CommandResponseOp& Op)
	{
		auto Response = Op.status_code == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS  ? ::improbable::database_sync::DatabaseSyncService::Commands::SetParent::Response(::improbable::database_sync::DatabaseSyncService::Commands::SetParent::Response::Type::Deserialize(Schema_GetCommandResponseObject(Op.response.schema_type))) : ::improbable::database_sync::DatabaseSyncService::Commands::SetParent::Response();
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::SetParent::ResponseOp(Op.entity_id, Op.request_id, Op.status_code, Op.message, Op.command_id, Response));
	});
}

Worker_RequestId ExternalSchemaInterface::SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::Create::Request& Request)
{
	return SerializeAndSendCommandRequest(EntityId, 1901, 6, Request.Data);
}

void ExternalSchemaInterface::SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::Create::Response& Response)
{
	SerializeAndSendCommandResponse(RequestId, 1901, 6, Response.Data);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Create::RequestOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandRequest(1901, [Callback](const Worker_CommandRequestOp& Op)
	{
		auto Request = ::improbable::database_sync::DatabaseSyncService::Commands::Create::Request(::improbable::database_sync::DatabaseSyncService::Commands::Create::Request::Type::Deserialize(Schema_GetCommandRequestObject(Op.request.schema_type)));
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::Create::RequestOp(Op.entity_id, Op.request_id, Op.timeout_millis, Op.caller_worker_id, Op.caller_attribute_set, Request));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Create::ResponseOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandResponse(1901, [Callback](const Worker_CommandResponseOp& Op)
	{
		auto Response = Op.status_code == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS  ? ::improbable::database_sync::DatabaseSyncService::Commands::Create::Response(::improbable::database_sync::DatabaseSyncService::Commands::Create::Response::Type::Deserialize(Schema_GetCommandResponseObject(Op.response.schema_type))) : ::improbable::database_sync::DatabaseSyncService::Commands::Create::Response();
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::Create::ResponseOp(Op.entity_id, Op.request_id, Op.status_code, Op.message, Op.command_id, Response));
	});
}

Worker_RequestId ExternalSchemaInterface::SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::Delete::Request& Request)
{
	return SerializeAndSendCommandRequest(EntityId, 1901, 7, Request.Data);
}

void ExternalSchemaInterface::SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::Delete::Response& Response)
{
	SerializeAndSendCommandResponse(RequestId, 1901, 7, Response.Data);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Delete::RequestOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandRequest(1901, [Callback](const Worker_CommandRequestOp& Op)
	{
		auto Request = ::improbable::database_sync::DatabaseSyncService::Commands::Delete::Request(::improbable::database_sync::DatabaseSyncService::Commands::Delete::Request::Type::Deserialize(Schema_GetCommandRequestObject(Op.request.schema_type)));
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::Delete::RequestOp(Op.entity_id, Op.request_id, Op.timeout_millis, Op.caller_worker_id, Op.caller_attribute_set, Request));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Delete::ResponseOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandResponse(1901, [Callback](const Worker_CommandResponseOp& Op)
	{
		auto Response = Op.status_code == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS  ? ::improbable::database_sync::DatabaseSyncService::Commands::Delete::Response(::improbable::database_sync::DatabaseSyncService::Commands::Delete::Response::Type::Deserialize(Schema_GetCommandResponseObject(Op.response.schema_type))) : ::improbable::database_sync::DatabaseSyncService::Commands::Delete::Response();
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::Delete::ResponseOp(Op.entity_id, Op.request_id, Op.status_code, Op.message, Op.command_id, Response));
	});
}

Worker_RequestId ExternalSchemaInterface::SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::Batch::Request& Request)
{
	return SerializeAndSendCommandRequest(EntityId, 1901, 8, Request.Data);
}

void ExternalSchemaInterface::SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::Batch::Response& Response)
{
	SerializeAndSendCommandResponse(RequestId, 1901, 8, Response.Data);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Batch::RequestOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandRequest(1901, [Callback](const Worker_CommandRequestOp& Op)
	{
		auto Request = ::improbable::database_sync::DatabaseSyncService::Commands::Batch::Request(::improbable::database_sync::DatabaseSyncService::Commands::Batch::Request::Type::Deserialize(Schema_GetCommandRequestObject(Op.request.schema_type)));
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::Batch::RequestOp(Op.entity_id, Op.request_id, Op.timeout_millis, Op.caller_worker_id, Op.caller_attribute_set, Request));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::Batch::ResponseOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandResponse(1901, [Callback](const Worker_CommandResponseOp& Op)
	{
		auto Response = Op.status_code == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS  ? ::improbable::database_sync::DatabaseSyncService::Commands::Batch::Response(::improbable::database_sync::DatabaseSyncService::Commands::Batch::Response::Type::Deserialize(Schema_GetCommandResponseObject(Op.response.schema_type))) : ::improbable::database_sync::DatabaseSyncService::Commands::Batch::Response();
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::Batch::ResponseOp(Op.entity_id, Op.request_id, Op.status_code, Op.message, Op.command_id, Response));
	});
}

Worker_RequestId ExternalSchemaInterface::SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::Request& Request)
{
	return SerializeAndSendCommandRequest(EntityId, 1901, 9, Request.Data);
}

void ExternalSchemaInterface::SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::Response& Response)
{
	SerializeAndSendCommandResponse(RequestId, 1901, 9, Response.Data);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::RequestOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandRequest(1901, [Callback](const Worker_CommandRequestOp& Op)
	{
		auto Request = ::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::Request(::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::Request::Type::Deserialize(Schema_GetCommandRequestObject(Op.request.schema_type)));
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::RequestOp(Op.entity_id, Op.request_id, Op.timeout_millis, Op.caller_worker_id, Op.caller_attribute_set, Request));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::ResponseOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandResponse(1901, [Callback](const Worker_CommandResponseOp& Op)
	{
		auto Response = Op.status_code == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS  ? ::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::Response(::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::Response::Type::Deserialize(Schema_GetCommandResponseObject(Op.response.schema_type))) : ::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::Response();
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::ResponseOp(Op.entity_id, Op.request_id, Op.status_code, Op.message, Op.command_id, Response));
	});
}

Worker_RequestId ExternalSchemaInterface::SendCommandRequest(Worker_EntityId EntityId, const ::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::Request& Request)
{
	return SerializeAndSendCommandRequest(EntityId, 1901, 10, Request.Data);
}

void ExternalSchemaInterface::SendCommandResponse(Worker_RequestId RequestId, const ::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::Response& Response)
{
	SerializeAndSendCommandResponse(RequestId, 1901, 10, Response.Data);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandRequest(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::RequestOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandRequest(1901, [Callback](const Worker_CommandRequestOp& Op)
	{
		auto Request = ::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::Request(::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::Request::Type::Deserialize(Schema_GetCommandRequestObject(Op.request.schema_type)));
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::RequestOp(Op.entity_id, Op.request_id, Op.timeout_millis, Op.caller_worker_id, Op.caller_attribute_set, Request));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnCommandResponse(const TFunction<void(const ::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::ResponseOp&)>& Callback)
{
	return SpatialDispatcher->OnCommandResponse(1901, [Callback](const Worker_CommandResponseOp& Op)
	{
		auto Response = Op.status_code == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS  ? ::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::Response(::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::Response::Type::Deserialize(Schema_GetCommandResponseObject(Op.response.schema_type))) : ::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::Response();
		Callback(::improbable::database_sync::DatabaseSyncService::Commands::GetMetrics::ResponseOp(Op.entity_id, Op.request_id, Op.status_code, Op.message, Op.command_id, Response));
	});
}

// Component improbable.database_sync.HydrationTest = 1903
void ExternalSchemaInterface::SendComponentUpdate(const Worker_EntityId EntityId, const ::improbable::database_sync::HydrationTest::Update& Update)
{             
	SerializeAndSendComponentUpdate(EntityId, 1903, Update);
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnAddComponent(const TFunction<void(const ::improbable::database_sync::HydrationTest::AddComponentOp&)>& Callback)
{
	return SpatialDispatcher->OnAddComponent(1903, [Callback](const Worker_AddComponentOp& Op)
	{
		::improbable::database_sync::HydrationTest Data = ::improbable::database_sync::HydrationTest::Deserialize(Op.data.schema_type);
		Callback(::improbable::database_sync::HydrationTest::AddComponentOp(Op.entity_id, Op.data.component_id, Data));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnRemoveComponent(const TFunction<void(const ::improbable::database_sync::HydrationTest::RemoveComponentOp&)>& Callback)
{
	return SpatialDispatcher->OnRemoveComponent(1903, [Callback](const Worker_RemoveComponentOp& Op)
	{
		Callback(::improbable::database_sync::HydrationTest::RemoveComponentOp(Op.entity_id, Op.component_id));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnComponentUpdate(const TFunction<void(const ::improbable::database_sync::HydrationTest::ComponentUpdateOp&)>& Callback)
{
	return SpatialDispatcher->OnComponentUpdate(1903, [Callback](const Worker_ComponentUpdateOp& Op)
	{
		::improbable::database_sync::HydrationTest::Update Update = ::improbable::database_sync::HydrationTest::Update::Deserialize(Op.update.schema_type);
		Callback(::improbable::database_sync::HydrationTest::ComponentUpdateOp(Op.entity_id, Op.update.component_id, Update));
	});
}

USpatialDispatcher::FCallbackId ExternalSchemaInterface::OnAuthorityChange(const TFunction<void(const ::improbable::database_sync::HydrationTest::AuthorityChangeOp&)>& Callback)
{
	return SpatialDispatcher->OnAuthorityChange(1903, [Callback](const Worker_AuthorityChangeOp& Op)
	{
		Callback(::improbable::database_sync::HydrationTest::AuthorityChangeOp(Op.entity_id, Op.component_id, static_cast<Worker_Authority>(Op.authority)));
	});
}

