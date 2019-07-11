﻿// Generated by Unreal External Schema Codegen

#pragma once

#include "CoreMinimal.h"
#include "Utils/SchemaOption.h"
#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "../../ExternalSchemaHelperFunctions.h"

#include "../../improbable/database_sync/DatabaseSyncItem.h"

namespace improbable {
namespace database_sync {

// Generated from C:\DEV\UnrealGDKExampleProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\dbsync\database_sync.schema(87,1)
class GetItemResponse : public improbable::SpatialType
{
public:
	// Creates a new instance with specified arguments for each field.
	GetItemResponse(const ::improbable::database_sync::DatabaseSyncItem& Item);
	// Creates a new instance with default values for each field.
	GetItemResponse();
	// Creates a new instance with default values for each field. This is
	// equivalent to a default-constructed instance.
	static GetItemResponse Create() { return {}; }
	// Copyable and movable.
	GetItemResponse(GetItemResponse&&) = default;
	GetItemResponse(const GetItemResponse&) = default;
	GetItemResponse& operator=(GetItemResponse&&) = default;
	GetItemResponse& operator=(const GetItemResponse&) = default;
	~GetItemResponse() = default;
	
	bool operator==(const GetItemResponse&) const;
	bool operator!=(const GetItemResponse&) const;
	
	// Serialize this object data into the C API argument
	void Serialize(Schema_Object* SchemaObject) const override;
	
	// Deserialize the C API object argument into an instance of this class and return it
	static GetItemResponse Deserialize(Schema_Object* SchemaObject);
	
	// Field Item = 1
	const ::improbable::database_sync::DatabaseSyncItem& GetItem() const;
	::improbable::database_sync::DatabaseSyncItem& GetItem();
	GetItemResponse& SetItem(const ::improbable::database_sync::DatabaseSyncItem&);
	
private:
	::improbable::database_sync::DatabaseSyncItem _Item;
};

inline uint32 GetTypeHash(const GetItemResponse& Value);

} // namespace database_sync
} // namespace improbable

