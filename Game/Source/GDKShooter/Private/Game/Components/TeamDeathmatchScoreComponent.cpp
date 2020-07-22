// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Game/Components/TeamDeathmatchScoreComponent.h"

#include "Net/UnrealNetwork.h"
#include "Runtime/Launch/Resources/Version.h"

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
#if ENGINE_MINOR_VERSION <= 24
	const int32 NewPlayerId = PlayerState->PlayerId;
#else
	const int32 NewPlayerId = PlayerState->GetPlayerId();
#endif

	if (!PlayerScoreMap.Contains(NewPlayerId))
	{
		if (const UTeamComponent* TeamComponent = PlayerState->FindComponentByClass<UTeamComponent>())
		{
			FPlayerScore NewPlayerScore;
			NewPlayerScore.PlayerId = NewPlayerId;
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
				NewTeamScore.TeamScore = 0;
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
#if ENGINE_MINOR_VERSION <= 24
	const int32 RemovedPlayerId = PlayerState->PlayerId;
#else
	const int32 RemovedPlayerId = PlayerState->GetPlayerId();
#endif

	if (PlayerScoreMap.Contains(RemovedPlayerId))
	{
		if (const UTeamComponent* TeamComponent = PlayerState->FindComponentByClass<UTeamComponent>())
		{
			const uint8 TeamId = TeamComponent->GetTeam().GetId();
			if (TeamScoreMap.Contains(TeamId))
			{
				TArray<FPlayerScore>* PlayerScores = &TeamScoreArray[TeamScoreMap[TeamId]].PlayerScores;
				PlayerScores->RemoveAt(PlayerScoreMap[RemovedPlayerId]);
				for (int i = 0; i < PlayerScores->Num(); i++)
				{
					int32 PlayerId = (*PlayerScores)[i].PlayerId;
					PlayerScoreMap[PlayerId] = i;
				}
			}
		}

		PlayerScoreMap.Remove(RemovedPlayerId);
	}
}

void UTeamDeathmatchScoreComponent::RecordKill(APlayerState* KillerState, APlayerState* VictimState)
{
#if ENGINE_MINOR_VERSION <= 24
	const int32 KillerId = KillerState->PlayerId;
	const int32 VictimId = VictimState->PlayerId;
#else
	const int32 KillerId = KillerState->GetPlayerId();
	const int32 VictimId = VictimState->GetPlayerId();
#endif

	if (PlayerScoreMap.Contains(KillerId))
	{
		const uint8 KillerTeamId = KillerState->FindComponentByClass<UTeamComponent>()->GetTeam().GetId();

		++TeamScoreArray[TeamScoreMap[KillerTeamId]].PlayerScores[PlayerScoreMap[KillerId]].Kills;
	}

	if (PlayerScoreMap.Contains(VictimId))
	{
		const uint8 VictimTeamId = VictimState->FindComponentByClass<UTeamComponent>()->GetTeam().GetId();

		++TeamScoreArray[TeamScoreMap[VictimTeamId]].PlayerScores[PlayerScoreMap[VictimId]].Deaths;
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

int32 UTeamDeathmatchScoreComponent::GetTeamScore(FGenericTeamId TeamId)
{
	if (TeamScoreMap.Contains(TeamId.GetId()))
	{
		return TeamScoreArray[TeamScoreMap[TeamId.GetId()]].TeamScore;
	}
	return -1;
}

void UTeamDeathmatchScoreComponent::SetTeamScore(FGenericTeamId TeamId, int32 TeamScore)
{
	if (TeamScoreMap.Contains(TeamId.GetId()))
	{
		TeamScoreArray[TeamScoreMap[TeamId.GetId()]].TeamScore = TeamScore;
	}
}
