﻿// Generated by Unreal External Schema Codegen

#include "CompositeResponse.h"
#include <set>
#include "../../MapEquals.h"

// Generated from C:\DEV\UnrealGDKExampleProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\dbsync\database_sync.schema(238,1)
namespace improbable {
namespace database_sync {

CompositeResponse::CompositeResponse(
	const SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementResponse>& Increment, 
	const SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementResponse>& Decrement, 
	const SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentResponse>& SetParent, 
	const SpatialGDK::TSchemaOption<::improbable::database_sync::CreateResponse>& Create, 
	const SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteResponse>& Delete, 
	const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemResponse>& GetItem, 
	const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsResponse>& GetItems)
: _Increment{ Increment }
, _Decrement{ Decrement }
, _SetParent{ SetParent }
, _Create{ Create }
, _Delete{ Delete }
, _GetItem{ GetItem }
, _GetItems{ GetItems } {}

CompositeResponse::CompositeResponse() {}

bool CompositeResponse::operator==(const CompositeResponse& Value) const
{
	return _Increment == Value._Increment && 
	_Decrement == Value._Decrement && 
	_SetParent == Value._SetParent && 
	_Create == Value._Create && 
	_Delete == Value._Delete && 
	_GetItem == Value._GetItem && 
	_GetItems == Value._GetItems;
}

bool CompositeResponse::operator!=(const CompositeResponse& Value) const
{
	return !operator== (Value);
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementResponse>& CompositeResponse::GetIncrement() const
{
	return _Increment;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementResponse>& CompositeResponse::GetIncrement()
{
	return _Increment; 
}

CompositeResponse& CompositeResponse::SetIncrement(const SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementResponse>& Value)
{
	_Increment = Value;
	return *this;
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementResponse>& CompositeResponse::GetDecrement() const
{
	return _Decrement;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementResponse>& CompositeResponse::GetDecrement()
{
	return _Decrement; 
}

CompositeResponse& CompositeResponse::SetDecrement(const SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementResponse>& Value)
{
	_Decrement = Value;
	return *this;
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentResponse>& CompositeResponse::GetSetParent() const
{
	return _SetParent;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentResponse>& CompositeResponse::GetSetParent()
{
	return _SetParent; 
}

CompositeResponse& CompositeResponse::SetSetParent(const SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentResponse>& Value)
{
	_SetParent = Value;
	return *this;
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::CreateResponse>& CompositeResponse::GetCreate() const
{
	return _Create;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::CreateResponse>& CompositeResponse::GetCreate()
{
	return _Create; 
}

CompositeResponse& CompositeResponse::SetCreate(const SpatialGDK::TSchemaOption<::improbable::database_sync::CreateResponse>& Value)
{
	_Create = Value;
	return *this;
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteResponse>& CompositeResponse::GetDelete() const
{
	return _Delete;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteResponse>& CompositeResponse::GetDelete()
{
	return _Delete; 
}

CompositeResponse& CompositeResponse::SetDelete(const SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteResponse>& Value)
{
	_Delete = Value;
	return *this;
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemResponse>& CompositeResponse::GetGetItem() const
{
	return _GetItem;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemResponse>& CompositeResponse::GetGetItem()
{
	return _GetItem; 
}

CompositeResponse& CompositeResponse::SetGetItem(const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemResponse>& Value)
{
	_GetItem = Value;
	return *this;
}

const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsResponse>& CompositeResponse::GetGetItems() const
{
	return _GetItems;
}

SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsResponse>& CompositeResponse::GetGetItems()
{
	return _GetItems; 
}

CompositeResponse& CompositeResponse::SetGetItems(const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsResponse>& Value)
{
	_GetItems = Value;
	return *this;
}

void CompositeResponse::Serialize(Schema_Object* SchemaObject) const
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

CompositeResponse CompositeResponse::Deserialize(Schema_Object* SchemaObject)
{
	CompositeResponse Data;
	
	// deserializing field Increment = 1
	Data._Increment = SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementResponse>(::improbable::database_sync::IncrementResponse::Deserialize(Schema_GetObject(SchemaObject, 1)));
	
	// deserializing field Decrement = 2
	Data._Decrement = SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementResponse>(::improbable::database_sync::DecrementResponse::Deserialize(Schema_GetObject(SchemaObject, 2)));
	
	// deserializing field SetParent = 3
	Data._SetParent = SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentResponse>(::improbable::database_sync::SetParentResponse::Deserialize(Schema_GetObject(SchemaObject, 3)));
	
	// deserializing field Create = 4
	Data._Create = SpatialGDK::TSchemaOption<::improbable::database_sync::CreateResponse>(::improbable::database_sync::CreateResponse::Deserialize(Schema_GetObject(SchemaObject, 4)));
	
	// deserializing field Delete = 5
	Data._Delete = SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteResponse>(::improbable::database_sync::DeleteResponse::Deserialize(Schema_GetObject(SchemaObject, 5)));
	
	// deserializing field GetItem = 6
	Data._GetItem = SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemResponse>(::improbable::database_sync::GetItemResponse::Deserialize(Schema_GetObject(SchemaObject, 6)));
	
	// deserializing field GetItems = 7
	Data._GetItems = SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsResponse>(::improbable::database_sync::GetItemsResponse::Deserialize(Schema_GetObject(SchemaObject, 7)));
	
	return Data;
}

uint32 GetTypeHash(const improbable::database_sync::CompositeResponse& Value)
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
