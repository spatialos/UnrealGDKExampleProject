﻿// Generated by Unreal External Schema Codegen

#pragma once

#include "CoreMinimal.h"
#include "Utils/SchemaOption.h"
#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "../../ExternalSchemaHelperFunctions.h"

#include "../../improbable/database_sync/IncrementResponse.h"
#include "../../improbable/database_sync/DecrementResponse.h"
#include "../../improbable/database_sync/SetParentResponse.h"
#include "../../improbable/database_sync/CreateResponse.h"
#include "../../improbable/database_sync/DeleteResponse.h"
#include "../../improbable/database_sync/GetItemResponse.h"
#include "../../improbable/database_sync/GetItemsResponse.h"

namespace improbable {
namespace database_sync {

// Generated from C:\DEV\UnrealGDKExampleProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\dbsync\database_sync.schema(238,1)
class CompositeResponse : public improbable::SpatialType
{
public:
	// Creates a new instance with specified arguments for each field.
	CompositeResponse(const SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementResponse>& Increment, const SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementResponse>& Decrement, const SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentResponse>& SetParent, const SpatialGDK::TSchemaOption<::improbable::database_sync::CreateResponse>& Create, const SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteResponse>& Delete, const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemResponse>& GetItem, const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsResponse>& GetItems);
	// Creates a new instance with default values for each field.
	CompositeResponse();
	// Creates a new instance with default values for each field. This is
	// equivalent to a default-constructed instance.
	static CompositeResponse Create() { return {}; }
	// Copyable and movable.
	CompositeResponse(CompositeResponse&&) = default;
	CompositeResponse(const CompositeResponse&) = default;
	CompositeResponse& operator=(CompositeResponse&&) = default;
	CompositeResponse& operator=(const CompositeResponse&) = default;
	~CompositeResponse() = default;
	
	bool operator==(const CompositeResponse&) const;
	bool operator!=(const CompositeResponse&) const;
	
	// Serialize this object data into the C API argument
	void Serialize(Schema_Object* SchemaObject) const override;
	
	// Deserialize the C API object argument into an instance of this class and return it
	static CompositeResponse Deserialize(Schema_Object* SchemaObject);
	
	// Field Increment = 1
	const SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementResponse>& GetIncrement() const;
	SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementResponse>& GetIncrement();
	CompositeResponse& SetIncrement(const SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementResponse>&);
	
	// Field Decrement = 2
	const SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementResponse>& GetDecrement() const;
	SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementResponse>& GetDecrement();
	CompositeResponse& SetDecrement(const SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementResponse>&);
	
	// Field SetParent = 3
	const SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentResponse>& GetSetParent() const;
	SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentResponse>& GetSetParent();
	CompositeResponse& SetSetParent(const SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentResponse>&);
	
	// Field Create = 4
	const SpatialGDK::TSchemaOption<::improbable::database_sync::CreateResponse>& GetCreate() const;
	SpatialGDK::TSchemaOption<::improbable::database_sync::CreateResponse>& GetCreate();
	CompositeResponse& SetCreate(const SpatialGDK::TSchemaOption<::improbable::database_sync::CreateResponse>&);
	
	// Field Delete = 5
	const SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteResponse>& GetDelete() const;
	SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteResponse>& GetDelete();
	CompositeResponse& SetDelete(const SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteResponse>&);
	
	// Field GetItem = 6
	const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemResponse>& GetGetItem() const;
	SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemResponse>& GetGetItem();
	CompositeResponse& SetGetItem(const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemResponse>&);
	
	// Field GetItems = 7
	const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsResponse>& GetGetItems() const;
	SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsResponse>& GetGetItems();
	CompositeResponse& SetGetItems(const SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsResponse>&);
	
private:
	SpatialGDK::TSchemaOption<::improbable::database_sync::IncrementResponse> _Increment;
	SpatialGDK::TSchemaOption<::improbable::database_sync::DecrementResponse> _Decrement;
	SpatialGDK::TSchemaOption<::improbable::database_sync::SetParentResponse> _SetParent;
	SpatialGDK::TSchemaOption<::improbable::database_sync::CreateResponse> _Create;
	SpatialGDK::TSchemaOption<::improbable::database_sync::DeleteResponse> _Delete;
	SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemResponse> _GetItem;
	SpatialGDK::TSchemaOption<::improbable::database_sync::GetItemsResponse> _GetItems;
};

inline uint32 GetTypeHash(const CompositeResponse& Value);

} // namespace database_sync
} // namespace improbable

