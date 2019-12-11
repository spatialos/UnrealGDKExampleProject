// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Game/Components/DeathmatchScoreComponent.h"
#include "Net/UnrealNetwork.h"

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
	if (!PlayerScoreMap.Contains(PlayerState->PlayerId))
	{
		FPlayerScore NewPlayerScore;
		NewPlayerScore.PlayerId = PlayerState->PlayerId;
		NewPlayerScore.PlayerName = PlayerState->GetPlayerName();
		NewPlayerScore.Kills = 0;
		NewPlayerScore.Deaths = 0;

		int32 Index = PlayerScoreArray.Add(NewPlayerScore);
		PlayerScoreMap.Emplace(NewPlayerScore.PlayerId, Index);
	}
}

void UDeathmatchScoreComponent::RecordKill(const int32 Killer, const int32 Victim)
{
	if (Killer != Victim && PlayerScoreMap.Contains(Killer))
	{
		++PlayerScoreArray[PlayerScoreMap[Killer]].Kills;
	}
	if (PlayerScoreMap.Contains(Victim))
	{
		++PlayerScoreArray[PlayerScoreMap[Victim]].Deaths;
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
