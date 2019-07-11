﻿// Generated by Unreal External Schema Codegen

#pragma once

#include "CoreMinimal.h"
#include "Utils/SchemaOption.h"
#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "../../ExternalSchemaHelperFunctions.h"

#include "../../improbable/database_sync/PathsUpdated.h"
#include "../../improbable/database_sync/GetItemRequest.h"
#include "../../improbable/database_sync/GetItemResponse.h"
#include "../../improbable/database_sync/GetItemsRequest.h"
#include "../../improbable/database_sync/GetItemsResponse.h"
#include "../../improbable/database_sync/IncrementRequest.h"
#include "../../improbable/database_sync/IncrementResponse.h"
#include "../../improbable/database_sync/DecrementRequest.h"
#include "../../improbable/database_sync/DecrementResponse.h"
#include "../../improbable/database_sync/SetParentRequest.h"
#include "../../improbable/database_sync/SetParentResponse.h"
#include "../../improbable/database_sync/CreateRequest.h"
#include "../../improbable/database_sync/CreateResponse.h"
#include "../../improbable/database_sync/DeleteRequest.h"
#include "../../improbable/database_sync/DeleteResponse.h"
#include "../../improbable/database_sync/BatchOperationRequest.h"
#include "../../improbable/database_sync/BatchOperationResponse.h"
#include "../../improbable/database_sync/AssociatePathWithClientRequest.h"
#include "../../improbable/database_sync/AssociatePathWithClientResponse.h"
#include "../../improbable/database_sync/GetMetricsRequest.h"
#include "../../improbable/database_sync/GetMetricsResponse.h"

namespace improbable {
namespace database_sync {

// Generated from C:\DEV\UnrealGDKExampleProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\dbsync\database_sync.schema(263,1)
class DatabaseSyncService : public improbable::SpatialComponent
{
public:
	static const Worker_ComponentId ComponentId = 1901;
	// Creates a new instance with specified arguments for each field.
	DatabaseSyncService(const TArray<FString>& WriteWorkerAttributes);
	// Creates a new instance with default values for each field.
	DatabaseSyncService();
	// Creates a new instance with default values for each field. This is
	// equivalent to a default-constructed instance.
	static DatabaseSyncService Create() { return {}; }
	// Copyable and movable.
	DatabaseSyncService(DatabaseSyncService&&) = default;
	DatabaseSyncService(const DatabaseSyncService&) = default;
	DatabaseSyncService& operator=(DatabaseSyncService&&) = default;
	DatabaseSyncService& operator=(const DatabaseSyncService&) = default;
	~DatabaseSyncService() = default;
	
	bool operator==(const DatabaseSyncService&) const;
	bool operator!=(const DatabaseSyncService&) const;
	
	// Serialize this object data into the C API argument
	void Serialize(Schema_ComponentData* ComponentData) const override;
	
	// Deserialize the C API object argument into an instance of this class and return it
	static DatabaseSyncService Deserialize(Schema_ComponentData* ComponentData);
	
	// Field WriteWorkerAttributes = 1
	const TArray<FString>& GetWriteWorkerAttributes() const;
	TArray<FString>& GetWriteWorkerAttributes();
	DatabaseSyncService& SetWriteWorkerAttributes(const TArray<FString>&);
	
private:
	TArray<FString> _WriteWorkerAttributes;

public:
	class Update : public improbable::SpatialComponentUpdate
	{
	public:
		// Creates a new instance with default values for each field.
		Update() = default;
		// Creates a new instance with default values for each field. This is
		// equivalent to a default-constructed instance.
		static Update Create() { return {}; }
		// Copyable and movable.
		Update(Update&&) = default;
		Update(const Update&) = default;
		Update& operator=(Update&&) = default;
		Update& operator=(const Update&) = default;
		~Update() = default;
		bool operator==(const Update&) const;
		bool operator!=(const Update&) const;
		
		// Creates an Update from a DatabaseSyncService object.
		static Update FromInitialData(const DatabaseSyncService& Data);
		
		/**
		 * Converts to a DatabaseSyncService
		 * object. It is an error to call this function unless *all* of the optional fields in this
		 * update are filled in.
		 */
		DatabaseSyncService ToInitialData() const;
		
		/**
		 * Replaces fields in the given DatabaseSyncService
		 * object with the corresponding fields in this update, where present.
		 */
		void ApplyTo(DatabaseSyncService&) const;
		
		// Serialize this update object data into the C API component update argument
		void Serialize(Schema_ComponentUpdate* ComponentUpdate) const override;
		
		// Deserialize the C API component update argument into an instance of this class and return it
		static Update Deserialize(Schema_ComponentUpdate* ComponentUpdate);
		
		// Field WriteWorkerAttributes = 1
		const SpatialGDK::TSchemaOption<TArray<FString>>& GetWriteWorkerAttributes() const;
		SpatialGDK::TSchemaOption<TArray<FString>>& GetWriteWorkerAttributes();
		DatabaseSyncService::Update& SetWriteWorkerAttributes(const TArray<FString>&);
		
		// Event PathsUpdated = 1
		const TArray<::improbable::database_sync::PathsUpdated>& GetPathsUpdatedList() const;
		TArray<::improbable::database_sync::PathsUpdated>& GetPathsUpdatedList();
		DatabaseSyncService::Update& AddPathsUpdated(const ::improbable::database_sync::PathsUpdated&);
		
	private:
		SpatialGDK::TSchemaOption<TArray<FString>> _WriteWorkerAttributes;
		TArray<::improbable::database_sync::PathsUpdated> _PathsUpdatedList;
	};

	class Commands
	{
	public:
		class GetItem
		{
		public:
			static const Schema_FieldId CommandIndex = 1;
			struct Request
			{
				using Type = ::improbable::database_sync::GetItemRequest;
				Request(const FString& Path, const FString& WorkerId)
				: Data(Path, WorkerId) {}
				Request(Type Data) : Data{ Data } {}
				Type Data;
			};
			
			struct Response
			{
				using Type = ::improbable::database_sync::GetItemResponse;
				Response(const ::improbable::database_sync::DatabaseSyncItem& Item)
				: Data(Item) {}
				Response(Type Data) : Data{ Data } {}
				Response() : Data() {}
				Type Data;
			};
			using RequestOp = ::improbable::CommandRequestOp<Request>;
			using ResponseOp = ::improbable::CommandResponseOp<Response>; 
		};
		class GetItems
		{
		public:
			static const Schema_FieldId CommandIndex = 2;
			struct Request
			{
				using Type = ::improbable::database_sync::GetItemsRequest;
				Request(const FString& Path, const ::improbable::database_sync::GetItemDepth& Depth, const FString& WorkerId)
				: Data(Path, Depth, WorkerId) {}
				Request(Type Data) : Data{ Data } {}
				Type Data;
			};
			
			struct Response
			{
				using Type = ::improbable::database_sync::GetItemsResponse;
				Response(const TArray<::improbable::database_sync::DatabaseSyncItem>& Items)
				: Data(Items) {}
				Response(Type Data) : Data{ Data } {}
				Response() : Data() {}
				Type Data;
			};
			using RequestOp = ::improbable::CommandRequestOp<Request>;
			using ResponseOp = ::improbable::CommandResponseOp<Response>; 
		};
		class Increment
		{
		public:
			static const Schema_FieldId CommandIndex = 3;
			struct Request
			{
				using Type = ::improbable::database_sync::IncrementRequest;
				Request(const FString& Path, int64 Amount, const FString& WorkerId)
				: Data(Path, Amount, WorkerId) {}
				Request(Type Data) : Data{ Data } {}
				Type Data;
			};
			
			struct Response
			{
				using Type = ::improbable::database_sync::IncrementResponse;
				Response(int64 NewCount)
				: Data(NewCount) {}
				Response(Type Data) : Data{ Data } {}
				Response() : Data() {}
				Type Data;
			};
			using RequestOp = ::improbable::CommandRequestOp<Request>;
			using ResponseOp = ::improbable::CommandResponseOp<Response>; 
		};
		class Decrement
		{
		public:
			static const Schema_FieldId CommandIndex = 4;
			struct Request
			{
				using Type = ::improbable::database_sync::DecrementRequest;
				Request(const FString& Path, int64 Amount, const FString& WorkerId)
				: Data(Path, Amount, WorkerId) {}
				Request(Type Data) : Data{ Data } {}
				Type Data;
			};
			
			struct Response
			{
				using Type = ::improbable::database_sync::DecrementResponse;
				Response(int64 NewCount)
				: Data(NewCount) {}
				Response(Type Data) : Data{ Data } {}
				Response() : Data() {}
				Type Data;
			};
			using RequestOp = ::improbable::CommandRequestOp<Request>;
			using ResponseOp = ::improbable::CommandResponseOp<Response>; 
		};
		class SetParent
		{
		public:
			static const Schema_FieldId CommandIndex = 5;
			struct Request
			{
				using Type = ::improbable::database_sync::SetParentRequest;
				Request(const FString& Path, const FString& NewParent, const FString& WorkerId)
				: Data(Path, NewParent, WorkerId) {}
				Request(Type Data) : Data{ Data } {}
				Type Data;
			};
			
			struct Response
			{
				using Type = ::improbable::database_sync::SetParentResponse;
				Response(const FString& NewPath, int64 ItemCount)
				: Data(NewPath, ItemCount) {}
				Response(Type Data) : Data{ Data } {}
				Response() : Data() {}
				Type Data;
			};
			using RequestOp = ::improbable::CommandRequestOp<Request>;
			using ResponseOp = ::improbable::CommandResponseOp<Response>; 
		};
		class Create
		{
		public:
			static const Schema_FieldId CommandIndex = 6;
			struct Request
			{
				using Type = ::improbable::database_sync::CreateRequest;
				Request(const ::improbable::database_sync::DatabaseSyncItem& Item, const FString& WorkerId)
				: Data(Item, WorkerId) {}
				Request(Type Data) : Data{ Data } {}
				Type Data;
			};
			
			struct Response
			{
				using Type = ::improbable::database_sync::CreateResponse;
				Response()
				: Data() {}
				Response(Type Data) : Data{ Data } {}
				Type Data;
			};
			using RequestOp = ::improbable::CommandRequestOp<Request>;
			using ResponseOp = ::improbable::CommandResponseOp<Response>; 
		};
		class Delete
		{
		public:
			static const Schema_FieldId CommandIndex = 7;
			struct Request
			{
				using Type = ::improbable::database_sync::DeleteRequest;
				Request(const FString& Path, const FString& WorkerId)
				: Data(Path, WorkerId) {}
				Request(Type Data) : Data{ Data } {}
				Type Data;
			};
			
			struct Response
			{
				using Type = ::improbable::database_sync::DeleteResponse;
				Response(int64 ItemCount)
				: Data(ItemCount) {}
				Response(Type Data) : Data{ Data } {}
				Response() : Data() {}
				Type Data;
			};
			using RequestOp = ::improbable::CommandRequestOp<Request>;
			using ResponseOp = ::improbable::CommandResponseOp<Response>; 
		};
		class Batch
		{
		public:
			static const Schema_FieldId CommandIndex = 8;
			struct Request
			{
				using Type = ::improbable::database_sync::BatchOperationRequest;
				Request(const TArray<::improbable::database_sync::CompositeRequest>& Actions)
				: Data(Actions) {}
				Request(Type Data) : Data{ Data } {}
				Type Data;
			};
			
			struct Response
			{
				using Type = ::improbable::database_sync::BatchOperationResponse;
				Response(const TArray<::improbable::database_sync::CompositeResponse>& Actions)
				: Data(Actions) {}
				Response(Type Data) : Data{ Data } {}
				Response() : Data() {}
				Type Data;
			};
			using RequestOp = ::improbable::CommandRequestOp<Request>;
			using ResponseOp = ::improbable::CommandResponseOp<Response>; 
		};
		class AssociatePathWithClient
		{
		public:
			static const Schema_FieldId CommandIndex = 9;
			struct Request
			{
				using Type = ::improbable::database_sync::AssociatePathWithClientRequest;
				Request(const FString& Path, const FString& ClientWorkerId)
				: Data(Path, ClientWorkerId) {}
				Request(Type Data) : Data{ Data } {}
				Type Data;
			};
			
			struct Response
			{
				using Type = ::improbable::database_sync::AssociatePathWithClientResponse;
				Response()
				: Data() {}
				Response(Type Data) : Data{ Data } {}
				Type Data;
			};
			using RequestOp = ::improbable::CommandRequestOp<Request>;
			using ResponseOp = ::improbable::CommandResponseOp<Response>; 
		};
		class GetMetrics
		{
		public:
			static const Schema_FieldId CommandIndex = 10;
			struct Request
			{
				using Type = ::improbable::database_sync::GetMetricsRequest;
				Request()
				: Data() {}
				Request(Type Data) : Data{ Data } {}
				Type Data;
			};
			
			struct Response
			{
				using Type = ::improbable::database_sync::GetMetricsResponse;
				Response(const TMap<uint32, int64>& CommandRequests, const TMap<FString, int64>& Queries, const TMap<FString, int64>& Database)
				: Data(CommandRequests, Queries, Database) {}
				Response(Type Data) : Data{ Data } {}
				Response() : Data() {}
				Type Data;
			};
			using RequestOp = ::improbable::CommandRequestOp<Request>;
			using ResponseOp = ::improbable::CommandResponseOp<Response>; 
		};
	};
	using AddComponentOp = ::improbable::AddComponentOp<DatabaseSyncService>;
	using RemoveComponentOp = ::improbable::RemoveComponentOp<DatabaseSyncService>;
	using ComponentUpdateOp = ::improbable::ComponentUpdateOp<Update>;
	using AuthorityChangeOp = ::improbable::AuthorityChangeOp<DatabaseSyncService>;
};

inline uint32 GetTypeHash(const DatabaseSyncService& Value);

} // namespace database_sync
} // namespace improbable

