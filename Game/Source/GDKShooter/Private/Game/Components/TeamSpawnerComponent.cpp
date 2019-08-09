// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TeamSpawnerComponent.h"

#include "Components/MetaDataComponent.h"
#include "Components/TeamComponent.h"
#include "EngineUtils.h"
#include "Components/PlayerPublisher.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GDKLogging.h"
#include "Runtime/AIModule/Classes/GenericTeamAgentInterface.h"

UTeamSpawnerComponent::UTeamSpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTeamSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	//Assign PlayerStart's to TeamId's
	int32 TeamId = CurrentTeamPointer;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if (PlayerStart->IsA<APlayerStartPIE>())
		{
			PlayerStartPIE = Cast<APlayerStartPIE>(PlayerStart);
		}
		else
		{
			TeamStartPoints.Add(TeamId, PlayerStart);
			TeamAssignments.Add(TeamId, 0);
			TeamId++;
		}
	}
}

int32 UTeamSpawnerComponent::GetAvailableTeamId()
{
	int32 AssignedTeam = CurrentTeamPointer;

	if (!TeamAssignments.Contains(CurrentTeamPointer))
	{
		TeamAssignments[CurrentTeamPointer] = 0;
	}

	TeamAssignments[CurrentTeamPointer]++;

	if (TeamAssignments[CurrentTeamPointer] == TeamCapacity)
	{
		CurrentTeamPointer++;
	}

	return AssignedTeam;
}

void UTeamSpawnerComponent::RequestSpawn(APlayerController* Controller)
{
	if (CurrentTeamPointer >= TeamAssignments.Num() && !PlayerStartPIE)
	{
		// We should be limiting the player count before we run out of teams
		return;
	}

	int32 TeamId = 255;

	if (SpawnedPlayers.Contains(Controller))
	{
		TeamId = SpawnedPlayers[Controller];
		// This is a respawn, we either grant another player character, or a spectator pawn
		if (!bAllowRespawning)
		{
			APlayerStart* PlayerStart = PlayerStartPIE ? PlayerStartPIE : TeamStartPoints[TeamId];

			if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
			{
				APawn* SpecPawn = nullptr;

				//Hardcoded path for the specbot BP, probably a better place to reference this
				static const FString Path = TEXT("Blueprint'/Game/Blueprints/SpecBot/BP_SpecBot.BP_SpecBot_C'");
				const TSubclassOf<AActor> BPSpecClass = Cast<UClass>(StaticLoadObject(UObject::StaticClass(), nullptr, *Path));

				if (BPSpecClass != nullptr)
				{
					FRotator StartRotation(ForceInit);
					StartRotation.Yaw = PlayerStart->GetActorRotation().Yaw;
					FVector StartLocation = PlayerStart->GetActorLocation();
					FTransform Transform = FTransform(StartRotation, StartLocation);

					FActorSpawnParameters SpawnInfo;
					SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					SpawnInfo.ObjectFlags |= RF_Transient;
					SpecPawn = Cast<APawn>(GetWorld()->SpawnActor(BPSpecClass, &Transform, SpawnInfo));
				}

				Controller->Possess(SpecPawn);
			}

			return;
		}
	}
	else
	{
		TeamId = GetAvailableTeamId();
		SpawnedPlayers.Add(Controller, TeamId);
	}
	
	APlayerStart* PlayerStart = PlayerStartPIE ? PlayerStartPIE : TeamStartPoints[TeamId];

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
