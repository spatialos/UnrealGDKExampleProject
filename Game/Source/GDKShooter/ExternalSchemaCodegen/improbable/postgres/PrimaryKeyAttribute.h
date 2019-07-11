﻿// Generated by Unreal External Schema Codegen

#pragma once

#include "CoreMinimal.h"
#include "Utils/SchemaOption.h"
#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "../../ExternalSchemaHelperFunctions.h"

namespace improbable {
namespace postgres {

// Generated from C:\DEV\UnrealGDKExampleProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\dbsync\postgres.schema(7,1)
class PrimaryKeyAttribute : public improbable::SpatialType
{
public:
	// Creates a new instance with default values for each field.
	PrimaryKeyAttribute();
	// Creates a new instance with default values for each field. This is
	// equivalent to a default-constructed instance.
	static PrimaryKeyAttribute Create() { return {}; }
	// Copyable and movable.
	PrimaryKeyAttribute(PrimaryKeyAttribute&&) = default;
	PrimaryKeyAttribute(const PrimaryKeyAttribute&) = default;
	PrimaryKeyAttribute& operator=(PrimaryKeyAttribute&&) = default;
	PrimaryKeyAttribute& operator=(const PrimaryKeyAttribute&) = default;
	~PrimaryKeyAttribute() = default;
	
	bool operator==(const PrimaryKeyAttribute&) const;
	bool operator!=(const PrimaryKeyAttribute&) const;
	
	// Serialize this object data into the C API argument
	void Serialize(Schema_Object* SchemaObject) const override;
	
	// Deserialize the C API object argument into an instance of this class and return it
	static PrimaryKeyAttribute Deserialize(Schema_Object* SchemaObject);
	
};

inline uint32 GetTypeHash(const PrimaryKeyAttribute& Value);

} // namespace postgres
} // namespace improbable

