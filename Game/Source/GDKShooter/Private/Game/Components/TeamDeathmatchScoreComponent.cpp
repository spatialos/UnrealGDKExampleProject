// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "Game/Components/TeamDeathmatchScoreComponent.h"
#include "Net/UnrealNetwork.h"
#include "Characters/Components/TeamComponent.h"

UTeamDeathmatchScoreComponent::UTeamDeathmatchScoreComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UTeamDeathmatchScoreComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTeamDeathmatchScoreComponent, TeamScoreArray);
}

void UTeamDeathmatchScoreComponent::SetTeamScores(TArray<FTeamScore> InitialTeamScores)
{
	for (FTeamScore Team : InitialTeamScores)
	{
		int32 TeamIndex = TeamScoreArray.Add(Team);
		TeamScoreMap.Emplace(Team.TeamId, TeamIndex);
	}
}

void UTeamDeathmatchScoreComponent::RecordNewPlayer(APlayerState* PlayerState)
{
	if (!PlayerScoreMap.Contains(PlayerState->PlayerId))
	{
		if (const UTeamComponent* TeamComponent = PlayerState->FindComponentByClass<UTeamComponent>())
		{
			FPlayerScore NewPlayerScore;
			NewPlayerScore.PlayerId = PlayerState->PlayerId;
			NewPlayerScore.PlayerName = PlayerState->GetPlayerName();
			NewPlayerScore.Kills = 0;
			NewPlayerScore.Deaths = 0;

			const uint8 TeamId = TeamComponent->GetTeam().GetId();
			if (!TeamScoreMap.Contains(TeamId))
			{
				UE_LOG(LogTemp, Error, TEXT("UTeamDeathmatchScoreComponent::RecordNewPlayer Attempted to record %s who is on an unknown team %d. Use SetTeamScores first."), *GetNameSafe(PlayerState), TeamId);
				// New Team.
				FTeamScore NewTeamScore;
				NewTeamScore.TeamId = TeamComponent->GetTeam();
				NewTeamScore.TeamName = FName(*FString::Printf(TEXT("Team %d"), NewTeamScore.TeamId));
				NewTeamScore.TotalKills = 0;
				NewTeamScore.TotalDeaths = 0;
				int32 PlayerIndex = NewTeamScore.PlayerScores.Add(NewPlayerScore);
				PlayerScoreMap.Emplace(NewPlayerScore.PlayerId, PlayerIndex);

				int32 TeamIndex = TeamScoreArray.Add(NewTeamScore);
				TeamScoreMap.Emplace(TeamId, TeamIndex);
			}
			else
			{
				// Existing Team.
				int32 PlayerIndex = TeamScoreArray[TeamScoreMap[TeamId]].PlayerScores.Add(NewPlayerScore);
				PlayerScoreMap.Emplace(NewPlayerScore.PlayerId, PlayerIndex);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("UTeamDeathmatchScoreComponent::RecordNewPlayer Attempted to record %s but no UTeamComponent found."), *GetNameSafe(PlayerState));
		}
	}
}

void UTeamDeathmatchScoreComponent::RemovePlayer(APlayerState* PlayerState)
{
	if (PlayerScoreMap.Contains(PlayerState->PlayerId))
	{
		if (const UTeamComponent* TeamComponent = PlayerState->FindComponentByClass<UTeamComponent>())
		{
			const uint8 TeamId = TeamComponent->GetTeam().GetId();
			if (TeamScoreMap.Contains(TeamId))
			{
				TArray<FPlayerScore>* PlayerScores = &TeamScoreArray[TeamScoreMap[TeamId]].PlayerScores;
				PlayerScores->RemoveAt(PlayerScoreMap[PlayerState->PlayerId]);
				for (int i = 0; i < PlayerScores->Num(); i++)
				{
					int32 PlayerId = (*PlayerScores)[i].PlayerId;
					PlayerScoreMap[PlayerId] = i;
				}
			}
		}

		PlayerScoreMap.Remove(PlayerState->PlayerId);
	}
}

void UTeamDeathmatchScoreComponent::RecordKill(APlayerState* KillerState, APlayerState* VictimState)
{
	if (PlayerScoreMap.Contains(KillerState->PlayerId))
	{
		const int32 KillerId = KillerState->PlayerId;
		const uint8 KillerTeamId = KillerState->FindComponentByClass<UTeamComponent>()->GetTeam().GetId();

		++TeamScoreArray[TeamScoreMap[KillerTeamId]].PlayerScores[PlayerScoreMap[KillerId]].Kills;
		++TeamScoreArray[TeamScoreMap[KillerTeamId]].TotalKills;
	}

	if (PlayerScoreMap.Contains(VictimState->PlayerId))
	{
		const int32 VictimId = VictimState->PlayerId;
		const uint8 VictimTeamId = VictimState->FindComponentByClass<UTeamComponent>()->GetTeam().GetId();

		++TeamScoreArray[TeamScoreMap[VictimTeamId]].PlayerScores[PlayerScoreMap[VictimId]].Deaths;
		++TeamScoreArray[TeamScoreMap[VictimTeamId]].TotalDeaths;
	}
}

void UTeamDeathmatchScoreComponent::OnRep_TeamScores()
{
	if (GetNetMode() == NM_Client)
	{
		for (int32 i = 0; i < TeamScoreArray.Num(); i++)
		{
			TeamScoreArray[i].PlayerScores.Sort([](const FPlayerScore& lhs, const FPlayerScore& rhs)
			{
				// Sort in reverse order.
				return lhs.Kills == rhs.Kills ? lhs.Deaths < rhs.Deaths : lhs.Kills > rhs.Kills;
			});
		}
	}

	ScoreEvent.Broadcast(TeamScoreArray);
}
