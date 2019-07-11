﻿// Generated by Unreal External Schema Codegen

#include "DecrementRequest.h"
#include <set>
#include "../../MapEquals.h"

// Generated from C:\DEV\UnrealGDKExampleProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\dbsync\database_sync.schema(136,1)
namespace improbable {
namespace database_sync {

DecrementRequest::DecrementRequest(
	const FString& Path, 
	int64 Amount, 
	const FString& WorkerId)
: _Path{ Path }
, _Amount{ Amount }
, _WorkerId{ WorkerId } {}

DecrementRequest::DecrementRequest() {}

bool DecrementRequest::operator==(const DecrementRequest& Value) const
{
	return _Path == Value._Path && 
	_Amount == Value._Amount && 
	_WorkerId == Value._WorkerId;
}

bool DecrementRequest::operator!=(const DecrementRequest& Value) const
{
	return !operator== (Value);
}

const FString& DecrementRequest::GetPath() const
{
	return _Path;
}

FString& DecrementRequest::GetPath()
{
	return _Path; 
}

DecrementRequest& DecrementRequest::SetPath(const FString& Value)
{
	_Path = Value;
	return *this;
}

int64 DecrementRequest::GetAmount() const
{
	return _Amount;
}

int64& DecrementRequest::GetAmount()
{
	return _Amount; 
}

DecrementRequest& DecrementRequest::SetAmount(int64 Value)
{
	_Amount = Value;
	return *this;
}

const FString& DecrementRequest::GetWorkerId() const
{
	return _WorkerId;
}

FString& DecrementRequest::GetWorkerId()
{
	return _WorkerId; 
}

DecrementRequest& DecrementRequest::SetWorkerId(const FString& Value)
{
	_WorkerId = Value;
	return *this;
}

void DecrementRequest::Serialize(Schema_Object* SchemaObject) const
{
	// serializing field Path = 1
	::improbable::utils::AddString(SchemaObject, 1, _Path);
	// serializing field Amount = 2
	Schema_AddInt64(SchemaObject, 2, _Amount);
	// serializing field WorkerId = 3
	::improbable::utils::AddString(SchemaObject, 3, _WorkerId);
}

DecrementRequest DecrementRequest::Deserialize(Schema_Object* SchemaObject)
{
	DecrementRequest Data;
	
	// deserializing field Path = 1
	Data._Path = ::improbable::utils::GetString(SchemaObject, 1);
	
	// deserializing field Amount = 2
	Data._Amount = Schema_GetInt64(SchemaObject, 2);
	
	// deserializing field WorkerId = 3
	Data._WorkerId = ::improbable::utils::GetString(SchemaObject, 3);
	
	return Data;
}

uint32 GetTypeHash(const improbable::database_sync::DecrementRequest& Value)
{
	uint32 Result = 1327;
	Result = (Result * 977) + ::GetTypeHash(Value.GetPath());
	Result = (Result * 977) + ::GetTypeHash(Value.GetAmount());
	Result = (Result * 977) + ::GetTypeHash(Value.GetWorkerId());
	return Result;
}

} // namespace database_sync
} // namespace improbable
