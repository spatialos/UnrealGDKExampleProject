﻿// Generated by Unreal External Schema Codegen

#include "CreateTableAttribute.h"
#include <set>
#include "../../MapEquals.h"

// Generated from C:\DEV\UnrealGDKExampleProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\dbsync\postgres.schema(3,1)
namespace improbable {
namespace postgres {

CreateTableAttribute::CreateTableAttribute(
	const TArray<FString>& Setup)
: _Setup{ Setup } {}

CreateTableAttribute::CreateTableAttribute() {}

bool CreateTableAttribute::operator==(const CreateTableAttribute& Value) const
{
	return _Setup == Value._Setup;
}

bool CreateTableAttribute::operator!=(const CreateTableAttribute& Value) const
{
	return !operator== (Value);
}

const TArray<FString>& CreateTableAttribute::GetSetup() const
{
	return _Setup;
}

TArray<FString>& CreateTableAttribute::GetSetup()
{
	return _Setup; 
}

CreateTableAttribute& CreateTableAttribute::SetSetup(const TArray<FString>& Value)
{
	_Setup = Value;
	return *this;
}

void CreateTableAttribute::Serialize(Schema_Object* SchemaObject) const
{
	// serializing field Setup = 1
	for (const FString& Element : _Setup)
	{
		::improbable::utils::AddString(SchemaObject, 1, Element);
	}
}

CreateTableAttribute CreateTableAttribute::Deserialize(Schema_Object* SchemaObject)
{
	CreateTableAttribute Data;
	
	// deserializing field Setup = 1
	{
		Data._Setup = ::improbable::utils::GetStringList(SchemaObject, 1);
	}
	
	return Data;
}

uint32 GetTypeHash(const improbable::postgres::CreateTableAttribute& Value)
{
	uint32 Result = 1327;
	for (const auto& item : Value.GetSetup())
	{
		Result = (Result * 977) + ::GetTypeHash(item);
	}
	return Result;
}

} // namespace postgres
} // namespace improbable
