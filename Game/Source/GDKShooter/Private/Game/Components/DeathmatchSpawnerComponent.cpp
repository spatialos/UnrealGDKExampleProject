// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Game/Components/DeathmatchSpawnerComponent.h"

#include "Characters/Components/MetaDataComponent.h"
#include "Characters/Components/TeamComponent.h"
#include "Controllers/GDKPlayerController.h"
#include "Engine/World.h"
#include "Game/Components/PlayerPublisher.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GDKLogging.h"
#include "Game/GDKPlayerStartBase.h"

UDeathmatchSpawnerComponent::UDeathmatchSpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UDeathmatchSpawnerComponent::RequestSpawn(APlayerController* Controller)
{
	// Spawn the Pawn
	SpawnCharacter(Controller);

	if (Controller->PlayerState)
	{
		if (UPlayerPublisher* PlayerPublisher = Cast<UPlayerPublisher>(GetWorld()->GetGameState()->GetComponentByClass(UPlayerPublisher::StaticClass())))
		{
			PlayerPublisher->PublishPlayer(Controller->PlayerState, EPlayerProgress::InGame);
		}
	}
}

AActor* UDeathmatchSpawnerComponent::GetSpawnPoint(APlayerController* Controller)
{
	AActor* NewStartSpot = GetWorld()->GetAuthGameMode()->ChoosePlayerStart(Controller);
	if (NewStartSpot != nullptr)
	{
		// Set the player controller / camera in this new location
		FRotator InitialControllerRot = NewStartSpot->GetActorRotation();
		InitialControllerRot.Roll = 0.f;
		Controller->SetInitialLocationAndRotation(NewStartSpot->GetActorLocation(), InitialControllerRot);
	}
	return NewStartSpot;
}

void UDeathmatchSpawnerComponent::SpawnCharacter(APlayerController* Controller)
{
	if (GetNetMode() == NM_Client)
	{
		UE_LOG(LogGDK, Error, TEXT("Attempting to call spawn player on a client."));
		return;
	}

	if (!bSpawningEnabled)
	{
		UE_LOG(LogGDK, Error, TEXT("Requested a player spawn while spawning was disabled."));
		return;
	}

	AActor* SpawnPoint = GetSpawnPoint(Controller);

	if (!SpawnPoint)
	{
		UE_LOG(LogGDK, Error, TEXT("Unable to find a valid spawn point."));
		return;
	}

	AGDKPlayerStartBase* PlayerStart = Cast<AGDKPlayerStartBase>(SpawnPoint);
	if (PlayerStart)
	{
		// Yuck... Have to tell the controller to migrate, and at this point, its spawn position tells where it is load balanced.
		if (auto GDKController = Cast<AGDKPlayerController>(Controller))
		{
			GDKController->SetSpawnLocation(SpawnPoint->GetActorLocation());
			GDKController->ForceNetUpdate();
		}
		PlayerStart->SpawnCharacter(Controller);
	}
}
