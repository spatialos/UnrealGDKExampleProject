// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "Game/Components/TeamDeathmatchSpawnerComponent.h"
#include "Characters/Components/MetaDataComponent.h"
#include "Characters/Components/TeamComponent.h"
#include "Components/ActorComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Game/Components/PlayerPublisher.h"
#include "GDKLogging.h"
#include "Math/NumericLimits.h"

DEFINE_LOG_CATEGORY(LogTeamDeathmatchSpawnerComponent)

UTeamDeathmatchSpawnerComponent::UTeamDeathmatchSpawnerComponent()
{	
	PrimaryComponentTick.bCanEverTick = false;
	bUseTeamPlayerStarts = true;
}

void UTeamDeathmatchSpawnerComponent::SetTeams(TArray<FGenericTeamId> TeamIds)
{
	for (FGenericTeamId TeamId : TeamIds)
	{
		TeamAssignments.Add(TeamId.GetId(), 0);
	}

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if (bUseTeamPlayerStarts)
		{
			if (UTeamComponent* TeamComponent = PlayerStart->FindComponentByClass<UTeamComponent>())
			{
				PlayerStarts.Add(PlayerStart);
			}
			else
			{
				UE_LOG(LogTeamDeathmatchSpawnerComponent, Log, TEXT("SetTeams ignoring %s as it does not have a TeamComponent"), *GetNameSafe(PlayerStart));
			}
		}
		else
		{
			PlayerStarts.Add(PlayerStart);
		}
	}
}

void UTeamDeathmatchSpawnerComponent::RequestSpawn(APlayerController* Controller)
{
	int32 TeamId = -1;
	if (SpawnedPlayers.Contains(Controller))
	{
		// Use player team.
		TeamId = SpawnedPlayers[Controller];
	}
	else
	{
		// Assign player to smallest team.
		TeamId = GetSmallestTeam();
		SpawnedPlayers.Add(Controller, TeamId);
		TeamAssignments[TeamId] += 1;
	}

	APlayerStart* PlayerStart = bUseTeamPlayerStarts ? GetNextTeamPlayerStart(FGenericTeamId(TeamId)) : GetNextPlayerStart();
	if (PlayerStart == nullptr)
	{
		UE_LOG(LogTeamDeathmatchSpawnerComponent, Error, TEXT("No player start available for %s (team %d)"), *GetNameSafe(Controller), TeamId);
		return;
	}

	if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
	{
		APawn* NewPawn = nullptr;

		NewPawn = GameMode->SpawnDefaultPawnFor(Controller, PlayerStart);

		Controller->Possess(NewPawn);

		if (!NewPawn)
		{
			UE_LOG(LogTeamDeathmatchSpawnerComponent, Error, TEXT("Null Pawn Returned from SpawnDefaultPawn"));
			return;
		}

		if (UMetaDataComponent* MetaDataComponent = Cast<UMetaDataComponent>(NewPawn->GetComponentByClass(UMetaDataComponent::StaticClass())))
		{
			FGDKMetaData MetaData;
			MetaData.Customization = TeamId;
			MetaDataComponent->SetMetaData(MetaData);
		}
		if (UTeamComponent* TeamComponent = Cast<UTeamComponent>(NewPawn->GetComponentByClass(UTeamComponent::StaticClass())))
		{
			TeamComponent->SetTeam(FGenericTeamId(TeamId));
		}
		else
		{
			UE_LOG(LogTeamDeathmatchSpawnerComponent, Error, TEXT("TeamComponent Required on Character"));
		}

		if (Controller->PlayerState != nullptr)
		{
			if (UTeamComponent* TeamComponent = Cast<UTeamComponent>(Controller->PlayerState->GetComponentByClass(UTeamComponent::StaticClass())))
			{
				TeamComponent->SetTeam(FGenericTeamId(TeamId));
			}
			else
			{
				UE_LOG(LogTeamDeathmatchSpawnerComponent, Error, TEXT("TeamComponent Required on PlayerState"));
			}

			if (UPlayerPublisher* PlayerPublisher = Cast<UPlayerPublisher>(GetWorld()->GetGameState()->GetComponentByClass(UPlayerPublisher::StaticClass())))
			{
				PlayerPublisher->PublishPlayer(Controller->PlayerState, EPlayerProgress::InGame);
			}
		}
	}
}

void UTeamDeathmatchSpawnerComponent::PlayerDisconnected(APlayerController* Controller)
{
	if (SpawnedPlayers.Contains(Controller))
	{
		TeamAssignments[SpawnedPlayers[Controller]] -= 1;
		SpawnedPlayers.Remove(Controller);
	}
}

int32 UTeamDeathmatchSpawnerComponent::GetSmallestTeam()
{
	int32 SmallestTeam = -1;
	int32 SmallestTeamSize = TNumericLimits<int32>::Max();

	for (auto& Entry : TeamAssignments)
	{
		if (Entry.Value < SmallestTeamSize)
		{
			SmallestTeamSize = Entry.Value;
			SmallestTeam = Entry.Key;
		}
	}

	return SmallestTeam;
}

APlayerStart* UTeamDeathmatchSpawnerComponent::GetNextTeamPlayerStart(FGenericTeamId Team)
{
	for (int i = 0; i < PlayerStarts.Num(); i++)
	{
		int index = (i + NextTeamPlayerStart.FindOrAdd(Team, 0)) % PlayerStarts.Num();
		if (const UTeamComponent* TeamComponent = PlayerStarts[index]->FindComponentByClass<UTeamComponent>())
		{
			if (TeamComponent->GetTeam() == Team)
			{
				NextTeamPlayerStart[Team] = index + 1;
				return PlayerStarts[index];
			}
		}
	}
	return nullptr;
}

APlayerStart* UTeamDeathmatchSpawnerComponent::GetNextPlayerStart()
{
	APlayerStart* PlayerStart = PlayerStarts[NextPlayerStart];
	NextPlayerStart = (NextPlayerStart + 1) % PlayerStarts.Num();
	
	return PlayerStart;
}
