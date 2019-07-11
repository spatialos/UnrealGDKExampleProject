// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DeathmatchScoreComponent.h"
#include "UnrealNetwork.h"
#include "SpatialNetDriver.h"
#include "GDKShooterSpatialGameInstance.h"
#include "ExternalSchemaCodegen/improbable/database_sync/CommandErrors.h"

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
	UE_LOG(LogTemp, Warning, TEXT("RecordNewPlayer"));

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

		// We have agreed that the Path we are storing the score is in the format "players.playerId.score.(AllTimeKills or AllTimeDeaths)"
		RequestGetItem("players." + FString::FromInt(PlayerState->PlayerId) + ".score.AllTimeKills"); // Get this value from persistent storage
		RequestGetItem("players." + FString::FromInt(PlayerState->PlayerId) + ".score.AllTimeDeaths"); // Get this value from persistent storage
	}
}

void UDeathmatchScoreComponent::RecordKill(const int32 Killer, const int32 Victim)
{
	if (Killer != Victim && PlayerScoreMap.Contains(Killer))
	{
		++PlayerScoreArray[PlayerScoreMap[Killer]].Kills;

		++PlayerScoreArray[PlayerScoreMap[Killer]].AllTimeKills;
		// We have agreed that the Path we are storing the score is in the format "players.playerId.score.(AllTimeKills or AllTimeDeaths)"
		RequestIncrement("players." + FString::FromInt(Killer) + ".score.AllTimeKills", 1);	// Store this value in persistent storage
	}
	if (PlayerScoreMap.Contains(Victim))
	{
		++PlayerScoreArray[PlayerScoreMap[Victim]].Deaths;

		++PlayerScoreArray[PlayerScoreMap[Victim]].AllTimeDeaths;
		// We have agreed that the Path we are storing the score is in the format "players.playerId.score.(AllTimeKills or AllTimeDeaths)"
		RequestIncrement("players." + FString::FromInt(Killer) + ".score.AllTimeDeaths", 1);	// Store this value in persistent storage
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
	UE_LOG(LogTemp, Warning, TEXT("Request Get Item"));

	FString workerId = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->GetWorkerId();

	::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Request* Request = new ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Request(Path, workerId);

	UGDKShooterSpatialGameInstance* gameInstance = Cast<UGDKShooterSpatialGameInstance>(GetWorld()->GetGameInstance());
	
	Worker_RequestId requestId = gameInstance->ExternalSchema->SendCommandRequest(gameInstance->HierarchyServiceId, *Request);

	GetItemRequests.Add(requestId, Request);

}

void UDeathmatchScoreComponent::GetItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::ResponseOp& Op)
{
	UE_LOG(LogTemp, Warning, TEXT("Get Item Response"));

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
	else
	{
		UGDKShooterSpatialGameInstance* gameInstance = Cast<UGDKShooterSpatialGameInstance>(GetWorld()->GetGameInstance());

		Worker_RequestId requestId = gameInstance->ExternalSchema->SendCommandRequest(gameInstance->HierarchyServiceId, *GetItemRequests[Op.RequestId]);

		GetItemRequests.Add(requestId, GetItemRequests[Op.RequestId]);

		GetItemRequests.Remove(Op.RequestId);
	}
}

void UDeathmatchScoreComponent::RequestCreateItem(const FString &Name, int64 Count, const FString &Path)
{
	FString workerId = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->GetWorkerId();

	::improbable::database_sync::DatabaseSyncItem* Item = new ::improbable::database_sync::DatabaseSyncItem(Name, Count, Path);

	::improbable::database_sync::DatabaseSyncService::Commands::Create::Request* Request = new ::improbable::database_sync::DatabaseSyncService::Commands::Create::Request(*Item, workerId);

	UGDKShooterSpatialGameInstance* gameInstance = Cast<UGDKShooterSpatialGameInstance>(GetWorld()->GetGameInstance());

	Worker_RequestId requestId = gameInstance->ExternalSchema->SendCommandRequest(gameInstance->HierarchyServiceId, *Request);

	CreateItemRequests.Add(requestId, Request);

}

void UDeathmatchScoreComponent::CreateItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Create::ResponseOp& Op)
{
	if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS)
	{
		CreateItemRequests.Remove(Op.RequestId);
	}
	else
	{
		UGDKShooterSpatialGameInstance* gameInstance = Cast<UGDKShooterSpatialGameInstance>(GetWorld()->GetGameInstance());

		Worker_RequestId requestId = gameInstance->ExternalSchema->SendCommandRequest(gameInstance->HierarchyServiceId, *CreateItemRequests[Op.RequestId]);

		CreateItemRequests.Add(requestId, CreateItemRequests[Op.RequestId]);

		CreateItemRequests.Remove(Op.RequestId);
	}
}


void UDeathmatchScoreComponent::RequestIncrement(const FString &Path, int64 Count)
{

	FString workerId = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->GetWorkerId();
		
	::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request* Request = new ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request(Path, Count, workerId);

	UGDKShooterSpatialGameInstance* gameInstance = Cast<UGDKShooterSpatialGameInstance>(GetWorld()->GetGameInstance());

	Worker_RequestId requestId = gameInstance->ExternalSchema->SendCommandRequest(gameInstance->HierarchyServiceId, *Request);

	IncrementRequests.Add(requestId, Request);
}

void UDeathmatchScoreComponent::IncrementResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::ResponseOp& Op)
{
	if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS)
	{
		IncrementRequests.Remove(Op.RequestId);
	}
	else
	{
		UGDKShooterSpatialGameInstance* gameInstance = Cast<UGDKShooterSpatialGameInstance>(GetWorld()->GetGameInstance());

		Worker_RequestId requestId = gameInstance->ExternalSchema->SendCommandRequest(gameInstance->HierarchyServiceId, *IncrementRequests[Op.RequestId]);

		IncrementRequests.Add(requestId, IncrementRequests[Op.RequestId]);

		IncrementRequests.Remove(Op.RequestId);
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
	// We have agreed that the Path we are storing the score is in the format "players.playerId.score.(AllTimeKills or AllTimeDeaths)"
	if (workingPath.RemoveFromStart("players."))
	{
		FString playerId;
		if (workingPath.Split(".", &playerId, &workingPath))
		{
			if (workingPath.RemoveFromStart("score."))
			{
				if (workingPath.Compare("AllTimeKills") == 0)
				{
					PlayerScoreArray[PlayerScoreMap[FCString::Atoi(*playerId)]].AllTimeKills = NewCount;
				}
				else if (workingPath.Compare("AllTimeDeaths") == 0)
				{
					PlayerScoreArray[PlayerScoreMap[FCString::Atoi(*playerId)]].AllTimeDeaths = NewCount;
				}
			}
		}
	}
}

void UDeathmatchScoreComponent::RequestCreateItemFromPath(const FString &Path)
{
	FString workingPath = *Path;
	// We have agreed that the Path we are storing the score is in the format "players.playerId.score.(AllTimeKills or AllTimeDeaths)"
	if (workingPath.RemoveFromStart("players."))
	{
		FString playerId;
		if (workingPath.Split(".", &playerId, &workingPath))
		{
			if (workingPath.RemoveFromStart("score."))
			{
				RequestCreateItem(workingPath, 0, Path);
			}
		}
	}
}
