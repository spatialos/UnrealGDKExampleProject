﻿// Generated by Unreal External Schema Codegen

#pragma once

#include "CoreMinimal.h"
#include "Utils/SchemaOption.h"
#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "../../ExternalSchemaHelperFunctions.h"

namespace improbable {
namespace database_sync {

// Generated from C:\DEV\UnrealGDKExampleProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\dbsync\database_sync.schema(196,1)
class DeleteRequest : public improbable::SpatialType
{
public:
	// Creates a new instance with specified arguments for each field.
	DeleteRequest(const FString& Path, const FString& WorkerId);
	// Creates a new instance with default values for each field.
	DeleteRequest();
	// Creates a new instance with default values for each field. This is
	// equivalent to a default-constructed instance.
	static DeleteRequest Create() { return {}; }
	// Copyable and movable.
	DeleteRequest(DeleteRequest&&) = default;
	DeleteRequest(const DeleteRequest&) = default;
	DeleteRequest& operator=(DeleteRequest&&) = default;
	DeleteRequest& operator=(const DeleteRequest&) = default;
	~DeleteRequest() = default;
	
	bool operator==(const DeleteRequest&) const;
	bool operator!=(const DeleteRequest&) const;
	
	// Serialize this object data into the C API argument
	void Serialize(Schema_Object* SchemaObject) const override;
	
	// Deserialize the C API object argument into an instance of this class and return it
	static DeleteRequest Deserialize(Schema_Object* SchemaObject);
	
	// Field Path = 1
	const FString& GetPath() const;
	FString& GetPath();
	DeleteRequest& SetPath(const FString&);
	
	// Field WorkerId = 2
	const FString& GetWorkerId() const;
	FString& GetWorkerId();
	DeleteRequest& SetWorkerId(const FString&);
	
private:
	FString _Path;
	FString _WorkerId;
};

inline uint32 GetTypeHash(const DeleteRequest& Value);

} // namespace database_sync
} // namespace improbable

