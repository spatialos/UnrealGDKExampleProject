// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DeathmatchScoreComponent.h"
#include "UnrealNetwork.h"
#include "SpatialNetDriver.h"
#include "ExternalSchemaCodegen/improbable/database_sync/CommandErrors.h"

// Path format to store the score is in the format "profiles.UnrealWorker.players.<playerId>.score.(AllTimeKills or AllTimeDeaths)"
namespace DBPaths
{
	static const FString kPlayersRoot = TEXT("profiles.UnrealWorker.players.");
	static const FString kScoreFolder = TEXT("score");
	static const FString kAllTimeKills = TEXT("AllTimeKills");
	static const FString kAllTimeDeaths = TEXT("AllTimeDeaths");
}

UDeathmatchScoreComponent::UDeathmatchScoreComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bReplicates = true;
}

void UDeathmatchScoreComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDeathmatchScoreComponent, PlayerScoreArray);
}

void UDeathmatchScoreComponent::RecordNewPlayer(APlayerState* PlayerState)
{
	if (GameInstance == nullptr)
	{
		GameInstance = Cast<UGDKShooterSpatialGameInstance>(GetWorld()->GetGameInstance());
	}

	if (!PlayerScoreMap.Contains(PlayerState->PlayerId))
	{
		FPlayerScore NewPlayerScore;
		NewPlayerScore.PlayerId = PlayerState->PlayerId;
		NewPlayerScore.PlayerName = PlayerState->GetPlayerName();
		NewPlayerScore.Kills = 0;
		NewPlayerScore.Deaths = 0;
		NewPlayerScore.AllTimeKills = 0;
		NewPlayerScore.AllTimeDeaths = 0;

		int32 Index = PlayerScoreArray.Add(NewPlayerScore);
		PlayerScoreMap.Emplace(NewPlayerScore.PlayerId, Index);

		// Only use the Database Sync Worker if the entity exists.
		if (GameInstance->GetHierarchyServiceId() != 0)
		{
			RequestGetItem(DBPaths::kPlayersRoot + FString::FromInt(PlayerState->PlayerId) + "." + DBPaths::kScoreFolder + "." + DBPaths::kAllTimeKills); // Get this value from persistent storage
			RequestGetItem(DBPaths::kPlayersRoot + FString::FromInt(PlayerState->PlayerId) + "." + DBPaths::kScoreFolder + "." + DBPaths::kAllTimeDeaths); // Get this value from persistent storage
		}
	}
}

void UDeathmatchScoreComponent::RecordKill(const int32 Killer, const int32 Victim)
{
	if (Killer != Victim && PlayerScoreMap.Contains(Killer))
	{
		++PlayerScoreArray[PlayerScoreMap[Killer]].Kills;

		++PlayerScoreArray[PlayerScoreMap[Killer]].AllTimeKills;
		// Only use the Database Sync Worker if the entity exists.
		if (GameInstance->GetHierarchyServiceId() != 0)
		{
			RequestIncrement(DBPaths::kPlayersRoot + FString::FromInt(Killer) + "." + DBPaths::kScoreFolder + "." + DBPaths::kAllTimeKills, 1);	// Store this value in persistent storage
		}
	}
	if (PlayerScoreMap.Contains(Victim))
	{
		++PlayerScoreArray[PlayerScoreMap[Victim]].Deaths;

		++PlayerScoreArray[PlayerScoreMap[Victim]].AllTimeDeaths;
		// Only use the Database Sync Worker if the entity exists.
		if (GameInstance->GetHierarchyServiceId() != 0)
		{
			RequestIncrement(DBPaths::kPlayersRoot + FString::FromInt(Victim) + "." + DBPaths::kScoreFolder + "." + DBPaths::kAllTimeDeaths, 1); // Store this value in persistent storage
		}
	}
}

void UDeathmatchScoreComponent::OnRep_PlayerScores()
{
	if (GetNetMode() == NM_Client)
	{
		// Re-sort player scores.
		PlayerScoreArray.Sort([](const FPlayerScore& lhs, const FPlayerScore& rhs)
		{
			// Sort in reverse order.
			return lhs.Kills == rhs.Kills ? lhs.Deaths < rhs.Deaths : lhs.Kills > rhs.Kills;
		});
	}

	ScoreEvent.Broadcast(PlayerScoreArray);
}

void UDeathmatchScoreComponent::RequestGetItem(const FString &Path)
{
	FString workerId = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->GetWorkerId();

	::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Request* Request = new ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Request(Path, workerId);
	
	Worker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), *Request);

	GetItemRequests.Add(requestId, Request);

}

void UDeathmatchScoreComponent::GetItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::ResponseOp& Op)
{
	if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS)
	{
		UpdateScoreFromPath(GetItemRequests[Op.RequestId]->Data.GetPath(), Op.Data.Data.GetItem().GetCount());		

		GetItemRequests.Remove(Op.RequestId);
	}
	else if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_APPLICATION_ERROR)
	{
		FString message = Op.Message;
		if (FCString::Atoi(*message) == (int32)::improbable::database_sync::CommandErrors::INVALID_REQUEST)
		{
			RequestCreateItemFromPath(GetItemRequests[Op.RequestId]->Data.GetPath());
		}
		
		GetItemRequests.Remove(Op.RequestId);
	}
	else if(Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_TIMEOUT)
	{
		Worker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), *GetItemRequests[Op.RequestId]);

		GetItemRequests.Add(requestId, GetItemRequests[Op.RequestId]);

		GetItemRequests.Remove(Op.RequestId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GetItem Request failed with Error %s : %s"), Op.StatusCode, Op.Message);
	}
}

void UDeathmatchScoreComponent::RequestCreateItem(const FString &Name, int64 Count, const FString &Path)
{
	FString workerId = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->GetWorkerId();

	::improbable::database_sync::DatabaseSyncItem* Item = new ::improbable::database_sync::DatabaseSyncItem(Name, Count, Path);

	::improbable::database_sync::DatabaseSyncService::Commands::Create::Request* Request = new ::improbable::database_sync::DatabaseSyncService::Commands::Create::Request(*Item, workerId);

	Worker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), *Request);

	CreateItemRequests.Add(requestId, Request);
}

void UDeathmatchScoreComponent::CreateItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Create::ResponseOp& Op)
{
	if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS)
	{
		CreateItemRequests.Remove(Op.RequestId);
	}
	else if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_TIMEOUT)
	{
		Worker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), *CreateItemRequests[Op.RequestId]);

		CreateItemRequests.Add(requestId, CreateItemRequests[Op.RequestId]);

		CreateItemRequests.Remove(Op.RequestId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CreateItem Request failed with Error %s : %s"), Op.StatusCode, Op.Message);
	}
}


void UDeathmatchScoreComponent::RequestIncrement(const FString &Path, int64 Count)
{

	FString workerId = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->GetWorkerId();
		
	::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request* Request = new ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request(Path, Count, workerId);

	Worker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), *Request);

	IncrementRequests.Add(requestId, Request);
}

void UDeathmatchScoreComponent::IncrementResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::ResponseOp& Op)
{
	if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS)
	{
		IncrementRequests.Remove(Op.RequestId);
	}
	else if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_TIMEOUT)
	{
		Worker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), *IncrementRequests[Op.RequestId]);

		IncrementRequests.Add(requestId, IncrementRequests[Op.RequestId]);

		IncrementRequests.Remove(Op.RequestId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Increment Request failed with Error %s : %s"), Op.StatusCode, Op.Message);
	}
}

void UDeathmatchScoreComponent::ItemUpdateEvent(const ::improbable::database_sync::DatabaseSyncService::ComponentUpdateOp& Op)
{
	for (int32 i = 0; i < Op.Update.GetPathsUpdatedList().Num(); i++)
	{
		for (int32 j = 0; j < Op.Update.GetPathsUpdatedList()[i].GetPaths().Num(); j++)
		{
			RequestGetItem(Op.Update.GetPathsUpdatedList()[i].GetPaths()[j]);
		}
	}
}

void UDeathmatchScoreComponent::UpdateScoreFromPath(const FString &Path, int64 NewCount)
{
	FString workingPath = *Path;
	if (workingPath.RemoveFromStart(DBPaths::kPlayersRoot))
	{
		FString playerId;
		if (workingPath.Split(".", &playerId, &workingPath))
		{
			if (workingPath.RemoveFromStart(DBPaths::kScoreFolder + "."))
			{
				if (workingPath.Compare(DBPaths::kAllTimeKills) == 0)
				{
					if (PlayerScoreMap.Contains(FCString::Atoi(*playerId)))
					{
						PlayerScoreArray[PlayerScoreMap[FCString::Atoi(*playerId)]].AllTimeKills = NewCount;
						return;
					}
					else
					{
						UE_LOG(LogTemp, Log, TEXT("Received Update from Player not currently in game, ignoring it."));
					}
				}
				else if (workingPath.Compare(DBPaths::kAllTimeDeaths) == 0)
				{
					if (PlayerScoreMap.Contains(FCString::Atoi(*playerId)))
					{
						PlayerScoreArray[PlayerScoreMap[FCString::Atoi(*playerId)]].AllTimeDeaths = NewCount;
						return;
					}
					else
					{
						UE_LOG(LogTemp, Log, TEXT("Received Update from Player not currently in game, ignoring it."));
					}
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("Received update with unexpected path : %s"), *Path);
}

void UDeathmatchScoreComponent::RequestCreateItemFromPath(const FString &Path)
{
	FString workingPath = *Path;
	if (workingPath.RemoveFromStart(DBPaths::kPlayersRoot))
	{
		FString playerId;
		if (workingPath.Split(".", &playerId, &workingPath))
		{
			if (workingPath.RemoveFromStart(DBPaths::kScoreFolder + "."))
			{
				RequestCreateItem(workingPath, 0, Path);
				return;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Request to create item from unexpected path : %s"), *Path);
}
