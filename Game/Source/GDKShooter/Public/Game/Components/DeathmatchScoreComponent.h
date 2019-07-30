// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/PlayerState.h"
#include "ExternalSchemaCodegen/improbable/database_sync/DatabaseSyncService.h"
#include "GDKShooterSpatialGameInstance.h"
#include "DeathmatchScoreComponent.generated.h"

// Information about a players performance during a match
USTRUCT(BlueprintType)
struct FPlayerScore {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
		int32 PlayerId;

	UPROPERTY(BlueprintReadOnly)
		FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
		int32 Kills;

	UPROPERTY(BlueprintReadOnly)
		int32 Deaths;

	UPROPERTY(BlueprintReadOnly)
		int32 AllTimeKills;

	UPROPERTY(BlueprintReadOnly)
		int32 AllTimeDeaths;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScoreChangeEvent, const TArray<FPlayerScore>&, LatestScores);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UDeathmatchScoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDeathmatchScoreComponent();

	UFUNCTION(BlueprintCallable)
		void RecordKill(int32 KillerId, int32 VictimId);

	UFUNCTION(BlueprintCallable)
		void RecordNewPlayer(APlayerState* PlayerState);

	UFUNCTION(BlueprintPure)
	TArray<FPlayerScore>& PlayerScores() { return PlayerScoreArray; }
	
	UPROPERTY(BlueprintAssignable)
		FScoreChangeEvent ScoreEvent;

	
	void ItemUpdateEvent(const ::improbable::database_sync::DatabaseSyncService::ComponentUpdateOp& Op);

	void GetItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::ResponseOp& Op);
	void CreateItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Create::ResponseOp& Op);
	void IncrementResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::ResponseOp& Op);

protected:
		
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
		void OnRep_PlayerScores();

	UPROPERTY(ReplicatedUsing = OnRep_PlayerScores)
		TArray<FPlayerScore> PlayerScoreArray;

	// A map from player name to score, to make it easier to update scores
	UPROPERTY()
		TMap<int32, int32> PlayerScoreMap;

	void RequestGetItem(const FString &Path);
	void RequestCreateItem(const FString &Name, int64 Count, const FString &Path);
	void RequestIncrement(const FString &Path, int64 Count);

	TMap<Worker_RequestId, ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Request> GetItemRequests;
	TMap<Worker_RequestId, ::improbable::database_sync::DatabaseSyncService::Commands::Create::Request> CreateItemRequests;
	TMap<Worker_RequestId, ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request> IncrementRequests;

	void UpdateScoreFromPath(const FString &Path, int64 NewCount);
	void RequestCreateItemFromPath(const FString &Path);

	UGDKShooterSpatialGameInstance* GameInstance = nullptr;
		
};
