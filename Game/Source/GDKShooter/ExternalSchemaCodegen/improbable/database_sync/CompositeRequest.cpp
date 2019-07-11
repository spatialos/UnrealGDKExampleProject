﻿// Generated by Unreal External Schema Codegen

#include "CompositeRequest.h"
#include <set>
#include "../../MapEquals.h"

// Generated from C:\DEV\UnrealGDKExampleProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\dbsync\database_sync.schema(227,1)
namespace improbable {
namespace database_sync {

CompositeRequest::CompositeRequest(
	const SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementRequest>& Increment, 
	const SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementRequest>& Decrement, 
	const SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentRequest>& SetParent, 
	const SpatialGDK::TSchemaOption<::improbable::database_sync::CreateRequest>& Create, 
	const SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteRequest>& Delete, 
	const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemRequest>& GetItem, 
	const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsRequest>& GetItems)
: _Increment{ Increment }
, _Decrement{ Decrement }
, _SetParent{ SetParent }
, _Create{ Create }
, _Delete{ Delete }
, _GetItem{ GetItem }
, _GetItems{ GetItems } {}

CompositeRequest::CompositeRequest() {}

bool CompositeRequest::operator==(const CompositeRequest& Value) const
{
	return _Increment == Value._Increment && 
	_Decrement == Value._Decrement && 
	_SetParent == Value._SetParent && 
	_Create == Value._Create && 
	_Delete == Value._Delete && 
	_GetItem == Value._GetItem && 
	_GetItems == Value._GetItems;
}

bool CompositeRequest::operator!=(const CompositeRequest& Value) const
{
	return !operator== (Value);
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementRequest>& CompositeRequest::GetIncrement() const
{
	return _Increment;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementRequest>& CompositeRequest::GetIncrement()
{
	return _Increment; 
}

CompositeRequest& CompositeRequest::SetIncrement(const SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementRequest>& Value)
{
	_Increment = Value;
	return *this;
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementRequest>& CompositeRequest::GetDecrement() const
{
	return _Decrement;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementRequest>& CompositeRequest::GetDecrement()
{
	return _Decrement; 
}

CompositeRequest& CompositeRequest::SetDecrement(const SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementRequest>& Value)
{
	_Decrement = Value;
	return *this;
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentRequest>& CompositeRequest::GetSetParent() const
{
	return _SetParent;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentRequest>& CompositeRequest::GetSetParent()
{
	return _SetParent; 
}

CompositeRequest& CompositeRequest::SetSetParent(const SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentRequest>& Value)
{
	_SetParent = Value;
	return *this;
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::CreateRequest>& CompositeRequest::GetCreate() const
{
	return _Create;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::CreateRequest>& CompositeRequest::GetCreate()
{
	return _Create; 
}

CompositeRequest& CompositeRequest::SetCreate(const SpatialGDK::TSchemaOption<::improbable::database_sync::CreateRequest>& Value)
{
	_Create = Value;
	return *this;
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteRequest>& CompositeRequest::GetDelete() const
{
	return _Delete;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteRequest>& CompositeRequest::GetDelete()
{
	return _Delete; 
}

CompositeRequest& CompositeRequest::SetDelete(const SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteRequest>& Value)
{
	_Delete = Value;
	return *this;
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemRequest>& CompositeRequest::GetGetItem() const
{
	return _GetItem;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemRequest>& CompositeRequest::GetGetItem()
{
	return _GetItem; 
}

CompositeRequest& CompositeRequest::SetGetItem(const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemRequest>& Value)
{
	_GetItem = Value;
	return *this;
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsRequest>& CompositeRequest::GetGetItems() const
{
	return _GetItems;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsRequest>& CompositeRequest::GetGetItems()
{
	return _GetItems; 
}

CompositeRequest& CompositeRequest::SetGetItems(const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsRequest>& Value)
{
	_GetItems = Value;
	return *this;
}

void CompositeRequest::Serialize(Schema_Object* SchemaObject) const
{
	// serializing field Increment = 1
	if (_Increment)
	{
		(*_Increment).Serialize(Schema_AddObject(SchemaObject, 1));
	}
	// serializing field Decrement = 2
	if (_Decrement)
	{
		(*_Decrement).Serialize(Schema_AddObject(SchemaObject, 2));
	}
	// serializing field SetParent = 3
	if (_SetParent)
	{
		(*_SetParent).Serialize(Schema_AddObject(SchemaObject, 3));
	}
	// serializing field Create = 4
	if (_Create)
	{
		(*_Create).Serialize(Schema_AddObject(SchemaObject, 4));
	}
	// serializing field Delete = 5
	if (_Delete)
	{
		(*_Delete).Serialize(Schema_AddObject(SchemaObject, 5));
	}
	// serializing field GetItem = 6
	if (_GetItem)
	{
		(*_GetItem).Serialize(Schema_AddObject(SchemaObject, 6));
	}
	// serializing field GetItems = 7
	if (_GetItems)
	{
		(*_GetItems).Serialize(Schema_AddObject(SchemaObject, 7));
	}
}

CompositeRequest CompositeRequest::Deserialize(Schema_Object* SchemaObject)
{
	CompositeRequest Data;
	
	// deserializing field Increment = 1
	Data._Increment = SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementRequest>(::improbable::database_sync::IncrementRequest::Deserialize(Schema_GetObject(SchemaObject, 1)));
	
	// deserializing field Decrement = 2
	Data._Decrement = SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementRequest>(::improbable::database_sync::DecrementRequest::Deserialize(Schema_GetObject(SchemaObject, 2)));
	
	// deserializing field SetParent = 3
	Data._SetParent = SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentRequest>(::improbable::database_sync::SetParentRequest::Deserialize(Schema_GetObject(SchemaObject, 3)));
	
	// deserializing field Create = 4
	Data._Create = SpatialGDK::TSchemaOption<::improbable::database_sync::CreateRequest>(::improbable::database_sync::CreateRequest::Deserialize(Schema_GetObject(SchemaObject, 4)));
	
	// deserializing field Delete = 5
	Data._Delete = SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteRequest>(::improbable::database_sync::DeleteRequest::Deserialize(Schema_GetObject(SchemaObject, 5)));
	
	// deserializing field GetItem = 6
	Data._GetItem = SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemRequest>(::improbable::database_sync::GetItemRequest::Deserialize(Schema_GetObject(SchemaObject, 6)));
	
	// deserializing field GetItems = 7
	Data._GetItems = SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsRequest>(::improbable::database_sync::GetItemsRequest::Deserialize(Schema_GetObject(SchemaObject, 7)));
	
	return Data;
}

uint32 GetTypeHash(const improbable::database_sync::CompositeRequest& Value)
{
	uint32 Result = 1327;
	Result = (Result * 977) + (Value.GetIncrement().IsSet() ? 1327u * improbable::database_sync::GetTypeHash(*Value.GetIncrement()) + 977u : 977u);
	Result = (Result * 977) + (Value.GetDecrement().IsSet() ? 1327u * improbable::database_sync::GetTypeHash(*Value.GetDecrement()) + 977u : 977u);
	Result = (Result * 977) + (Value.GetSetParent().IsSet() ? 1327u * improbable::database_sync::GetTypeHash(*Value.GetSetParent()) + 977u : 977u);
	Result = (Result * 977) + (Value.GetCreate().IsSet() ? 1327u * improbable::database_sync::GetTypeHash(*Value.GetCreate()) + 977u : 977u);
	Result = (Result * 977) + (Value.GetDelete().IsSet() ? 1327u * improbable::database_sync::GetTypeHash(*Value.GetDelete()) + 977u : 977u);
	Result = (Result * 977) + (Value.GetGetItem().IsSet() ? 1327u * improbable::database_sync::GetTypeHash(*Value.GetGetItem()) + 977u : 977u);
	Result = (Result * 977) + (Value.GetGetItems().IsSet() ? 1327u * improbable::database_sync::GetTypeHash(*Value.GetGetItems()) + 977u : 977u);
	return Result;
}

} // namespace database_sync
} // namespace improbable
