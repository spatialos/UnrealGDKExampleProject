// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKGameState.h"

#include "GDKLogging.h"
#include "UnrealNetwork.h"

AGDKGameState::AGDKGameState()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGDKGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGDKGameState, PlayerScoreArray);
	DOREPLIFETIME(AGDKGameState, ConnectedPlayers);
}

void AGDKGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (GetNetMode() != NM_Client)
	{
		ConnectedPlayers = PlayerArray.Num();
	}
}

void AGDKGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	if (GetNetMode() != NM_Client)
	{
		ConnectedPlayers = PlayerArray.Num();
	}
}

void AGDKGameState::AddPlayer(const int32 Player, const FString& PlayerName)
{
	// Start game ticking once the first player joins
	// Could later make this wait for a certain number of players

	if(PlayerScoreMap.Contains(Player))
	{
		// Player already added
		return;
	}

	AddPlayerInternal(Player, PlayerName);
}

void AGDKGameState::AddDeath(const int32 Killer, const int32 Victim)
{
	if (Killer != Victim && PlayerScoreMap.Contains(Killer))
	{
		++PlayerScoreArray[PlayerScoreMap[Killer]].Kills;
	}
	if (PlayerScoreMap.Contains(Victim))
	{
		++PlayerScoreArray[PlayerScoreMap[Victim]].Deaths;
	}


	//MulticastNotifyKill(Killer, PlayerScores[Killer]->PlayerName, Victim, PlayerScores[Victim]->PlayerName);
}

void AGDKGameState::OnRep_PlayerScores()
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

void AGDKGameState::OnRep_ConnectedPlayers()
{
	PlayerCountEvent.Broadcast(ConnectedPlayers);
}

void AGDKGameState::AddPlayerInternal(const int32 Player, const FString& PlayerName)
{
	FPlayerScore NewPlayerScore;
	NewPlayerScore.PlayerId = Player;
	NewPlayerScore.PlayerName = PlayerName;
	NewPlayerScore.Kills = 0;
	NewPlayerScore.Deaths = 0;

	int32 Index = PlayerScoreArray.Add(NewPlayerScore);
	PlayerScoreMap.Emplace(Player, Index);
}

void AGDKGameState::MulticastNotifyKill_Implementation(int32 KillerId, const FString& KillerName, int32 VictimId, const FString& VictimName)
{
	if (GetNetMode() == NM_Client)
	{
		//Not currently implemented
		//KillNotificationCallback.ExecuteIfBound(KillerId, KillerName, VictimId, VictimName);
	}
}



