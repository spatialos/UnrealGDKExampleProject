﻿// Generated by Unreal External Schema Codegen

#include "FieldTypeAttribute.h"
#include <set>
#include "../../MapEquals.h"

// Generated from C:\DEV\UnrealGDKExampleProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\dbsync\postgres.schema(13,1)
namespace improbable {
namespace postgres {

FieldTypeAttribute::FieldTypeAttribute(
	const FString& Type)
: _Type{ Type } {}

FieldTypeAttribute::FieldTypeAttribute() {}

bool FieldTypeAttribute::operator==(const FieldTypeAttribute& Value) const
{
	return _Type == Value._Type;
}

bool FieldTypeAttribute::operator!=(const FieldTypeAttribute& Value) const
{
	return !operator== (Value);
}

const FString& FieldTypeAttribute::GetType() const
{
	return _Type;
}

FString& FieldTypeAttribute::GetType()
{
	return _Type; 
}

FieldTypeAttribute& FieldTypeAttribute::SetType(const FString& Value)
{
	_Type = Value;
	return *this;
}

void FieldTypeAttribute::Serialize(Schema_Object* SchemaObject) const
{
	// serializing field Type = 1
	::improbable::utils::AddString(SchemaObject, 1, _Type);
}

FieldTypeAttribute FieldTypeAttribute::Deserialize(Schema_Object* SchemaObject)
{
	FieldTypeAttribute Data;
	
	// deserializing field Type = 1
	Data._Type = ::improbable::utils::GetString(SchemaObject, 1);
	
	return Data;
}

uint32 GetTypeHash(const improbable::postgres::FieldTypeAttribute& Value)
{
	uint32 Result = 1327;
	Result = (Result * 977) + ::GetTypeHash(Value.GetType());
	return Result;
}

} // namespace postgres
} // namespace improbable
