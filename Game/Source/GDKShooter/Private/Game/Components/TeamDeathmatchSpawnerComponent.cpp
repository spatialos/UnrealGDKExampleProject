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

UTeamDeathmatchSpawnerComponent::UTeamDeathmatchSpawnerComponent()
{	
	PrimaryComponentTick.bCanEverTick = false;
}

void UTeamDeathmatchSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	for (int i = 0; i < NumTeams; i++)
	{
		TeamAssignments.Add(i, 0);
	}

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		PlayerStarts.Add(PlayerStart);
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

	APlayerStart* PlayerStart = PlayerStarts[NextPlayerStart];
	NextPlayerStart = (NextPlayerStart + 1) % PlayerStarts.Num();

	if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
	{
		APawn* NewPawn = nullptr;

		NewPawn = GameMode->SpawnDefaultPawnFor(Controller, PlayerStart);

		Controller->Possess(NewPawn);

		if (!NewPawn)
		{
			UE_LOG(LogGDK, Error, TEXT("Null Pawn Returned from SpawnDefaultPawn"));
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
			UE_LOG(LogGDK, Error, TEXT("TeamComponent Required on Character"));
		}

		if (Controller->PlayerState != nullptr)
		{
			if (UTeamComponent* TeamComponent = Cast<UTeamComponent>(Controller->PlayerState->GetComponentByClass(UTeamComponent::StaticClass())))
			{
				TeamComponent->SetTeam(FGenericTeamId(TeamId));
			}
			else
			{
				UE_LOG(LogGDK, Error, TEXT("TeamComponent Required on PlayerState"));
			}

			if (UPlayerPublisher* PlayerPublisher = Cast<UPlayerPublisher>(GetWorld()->GetGameState()->GetComponentByClass(UPlayerPublisher::StaticClass())))
			{
				PlayerPublisher->PublishPlayer(Controller->PlayerState, EPlayerProgress::InGame);
			}
		}
	}
}

int32 UTeamDeathmatchSpawnerComponent::GetSmallestTeam()
{
	int32 SmallestTeam = 0;
	int32 SmallestTeamSize = TeamAssignments[SmallestTeam];
	for (int i = 1; i < NumTeams; i++)
	{
		if (TeamAssignments[i] < SmallestTeamSize)
		{
			SmallestTeamSize = TeamAssignments[i];
			SmallestTeam = i;
		}
	}
	return SmallestTeam;
}
