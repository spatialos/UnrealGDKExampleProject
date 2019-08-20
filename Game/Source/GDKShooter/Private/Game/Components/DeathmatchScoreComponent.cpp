// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DeathmatchScoreComponent.h"
#include "UnrealNetwork.h"

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
	if (!PlayerScoreMap.Contains(PlayerState->GetPlayerName()))
	{
		FPlayerScore NewPlayerScore;
		NewPlayerScore.PlayerId = PlayerState->PlayerId;
		NewPlayerScore.PlayerName = PlayerState->GetPlayerName();
		NewPlayerScore.Kills = 0;
		NewPlayerScore.Deaths = 0;
		
		NewPlayerScore.AllTimeKills = 0;	// TUTORIAL-TODO: Get this value from persistent storage
		NewPlayerScore.AllTimeDeaths = 0;	// TUTORIAL-TODO: Get this value from persistene storage

		int32 Index = PlayerScoreArray.Add(NewPlayerScore);
		PlayerScoreMap.Emplace(NewPlayerScore.PlayerName, Index);
	}
}

void UDeathmatchScoreComponent::RecordKill(const FString Killer, const FString Victim)
{
	if (Killer != Victim && PlayerScoreMap.Contains(Killer))
	{
		++PlayerScoreArray[PlayerScoreMap[Killer]].Kills;

		++PlayerScoreArray[PlayerScoreMap[Killer]].AllTimeKills;	// TUTORIAL-TODO: Store this value in persistent storage
	}
	if (PlayerScoreMap.Contains(Victim))
	{
		++PlayerScoreArray[PlayerScoreMap[Victim]].Deaths;

		++PlayerScoreArray[PlayerScoreMap[Victim]].AllTimeDeaths;	// TUTORIAL-TODO: Store this value in persistent storage
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
