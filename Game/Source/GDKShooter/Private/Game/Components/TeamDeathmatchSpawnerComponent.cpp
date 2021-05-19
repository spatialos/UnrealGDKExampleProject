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
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY(LogTeamDeathmatchSpawnerComponent)

UTeamDeathmatchSpawnerComponent::UTeamDeathmatchSpawnerComponent()
{	
	PrimaryComponentTick.bCanEverTick = false;
	bUseTeamPlayerStarts = true;
	bShufflePlayerStarts = true;
	NextPlayerStart = 0;
	SetIsReplicatedByDefault(true);
}

void UTeamDeathmatchSpawnerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTeamDeathmatchSpawnerComponent, PlayerStarts);
	DOREPLIFETIME(UTeamDeathmatchSpawnerComponent, TeamPlayerStarts);
	DOREPLIFETIME(UTeamDeathmatchSpawnerComponent, TeamAssignments);
}

void UTeamDeathmatchSpawnerComponent::SetTeams(TArray<FGenericTeamId> TeamIds)
{
	for (FGenericTeamId TeamId : TeamIds)
	{
		TeamAssignments.Add(FTeamAssignment(TeamId.GetId(), 0));
	}

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if (bUseTeamPlayerStarts)
		{
			if (UTeamComponent* TeamComponent = PlayerStart->FindComponentByClass<UTeamComponent>())
			{
				TeamPlayerStarts.Add(PlayerStart);
			}
			else
			{
				PlayerStarts.Add(PlayerStart);
			}
		}
		else
		{
			PlayerStarts.Add(PlayerStart);
		}
	}

	if (bUseTeamPlayerStarts)
	{
		ShufflePlayerStartArray(TeamPlayerStarts);
	}

	ShufflePlayerStartArray(PlayerStarts);
}

void UTeamDeathmatchSpawnerComponent::RequestSpawn(APlayerController* Controller)
{
	if (TeamAssignments.Num() == 0)
	{
		UE_LOG(LogTeamDeathmatchSpawnerComponent, Error, TEXT("Requested spawn but Spawner component hasn't been initialized! Controller: %s"), *GetNameSafe(Controller));
		return;
	}

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
		for (FTeamAssignment& TA : TeamAssignments)
		{
			if (TA.TeamId == TeamId)
			{
				TA.PlayerNum++;
				break;;
			}
		}
	}
	APlayerStart* PlayerStart = nullptr;

	if (bUseTeamPlayerStarts)
	{
		PlayerStart = GetNextTeamPlayerStart(FGenericTeamId(TeamId));
	}
	if (PlayerStart == nullptr)
	{
		PlayerStart = GetNextPlayerStart();
	}
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
		int32 TeamId = SpawnedPlayers[Controller];
		for (FTeamAssignment& TA : TeamAssignments)
		{
			if (TA.TeamId == TeamId)
			{
				TA.PlayerNum--;
				break;;
			}
		}
		SpawnedPlayers.Remove(Controller);
	}
}

int32 UTeamDeathmatchSpawnerComponent::GetSmallestTeam()
{
	int32 SmallestTeam = -1;
	int32 SmallestTeamSize = TNumericLimits<int32>::Max();

	for (FTeamAssignment& Entry : TeamAssignments)
	{
		if (Entry.PlayerNum < SmallestTeamSize)
		{
			SmallestTeamSize = Entry.PlayerNum;
			SmallestTeam = Entry.TeamId;
		}
	}

	return SmallestTeam;
}

APlayerStart* UTeamDeathmatchSpawnerComponent::GetNextTeamPlayerStart(FGenericTeamId Team)
{
	for (int i = 0; i < TeamPlayerStarts.Num(); i++)
	{
		int index = (i + NextTeamPlayerStart.FindOrAdd(Team, 0)) % TeamPlayerStarts.Num();
		if (const UTeamComponent* TeamComponent = TeamPlayerStarts[index]->FindComponentByClass<UTeamComponent>())
		{
			if (TeamComponent->GetTeam() == Team)
			{
				NextTeamPlayerStart[Team] = index + 1;
				return TeamPlayerStarts[index];
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


void UTeamDeathmatchSpawnerComponent::OnRep_Teams()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_Teams"));
}

void UTeamDeathmatchSpawnerComponent::ShufflePlayerStartArray(TArray<APlayerStart*> Array)
{
	int32 LastIndex = Array.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 Index = FMath::RandRange(i, LastIndex);
		if (i != Index)
		{
			Array.Swap(i, Index);
		}
	}
}
