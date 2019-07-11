﻿// Generated by Unreal External Schema Codegen

#pragma once

#include "CoreMinimal.h"
#include "Utils/SchemaOption.h"
#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "../../ExternalSchemaHelperFunctions.h"

namespace improbable {
namespace database_sync {

// Generated from C:\DEV\UnrealGDKExampleProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\dbsync\database_sync.schema(194,1)
class CreateResponse : public improbable::SpatialType
{
public:
	// Creates a new instance with default values for each field.
	CreateResponse();
	// Creates a new instance with default values for each field. This is
	// equivalent to a default-constructed instance.
	static CreateResponse Create() { return {}; }
	// Copyable and movable.
	CreateResponse(CreateResponse&&) = default;
	CreateResponse(const CreateResponse&) = default;
	CreateResponse& operator=(CreateResponse&&) = default;
	CreateResponse& operator=(const CreateResponse&) = default;
	~CreateResponse() = default;
	
	bool operator==(const CreateResponse&) const;
	bool operator!=(const CreateResponse&) const;
	
	// Serialize this object data into the C API argument
	void Serialize(Schema_Object* SchemaObject) const override;
	
	// Deserialize the C API object argument into an instance of this class and return it
	static CreateResponse Deserialize(Schema_Object* SchemaObject);
	
};

inline uint32 GetTypeHash(const CreateResponse& Value);

} // namespace database_sync
} // namespace improbable

