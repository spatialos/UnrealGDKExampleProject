// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Controllers/GDKPlayerController.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Blueprint/UserWidget.h"
#include "UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/HealthComponent.h"
#include "Components/MetaDataComponent.h"
#include "Components/EquippedComponent.h"
#include "Weapons/Holdable.h"
#include "Game/Components/ScorePublisher.h"
#include "Game/Components/PlayerPublisher.h"
#include "Game/Components/SpawnRequestPublisher.h"
#include "SpatialNetDriver.h"
#include "Connection/SpatialWorkerConnection.h"


AGDKPlayerController::AGDKPlayerController()
	: bIgnoreActionInput(false)
	, DeleteCharacterDelay(5.0f)
{
	// Don't automatically switch the camera view when the pawn changes, to avoid weird camera jumps when a character dies.
	bAutoManageActiveCameraTarget = false;

	// Create a camera boom (pulls in towards the controller if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create the third person camera
	DeathCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DeathCamera"));
	DeathCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	DeathCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

}

void AGDKPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerState)
	{
		if (UPlayerPublisher* PlayerPublisher = Cast<UPlayerPublisher>(GetWorld()->GetGameState()->GetComponentByClass(UPlayerPublisher::StaticClass())))
		{
			PlayerPublisher->PublishPlayer(PlayerState, EPlayerProgress::Connected);
		}
	}

	if (bAutoConnect && HasAuthority())
	{
		FGDKMetaData MetaData;
		MetaData.Customization = 0;
		ServerTryJoinGame("", MetaData);
	}
}

void AGDKPlayerController::Tick(float DeltaTime)
{
	if (GetPawn())
	{
		LatestPawnYaw = GetPawn()->GetActorRotation().Yaw;
	}
}

void AGDKPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (GetNetMode() == NM_Client && InPawn)
	{
		SetViewTarget(InPawn);
		// Make the new pawn's camera this controller's camera.
		this->ClientSetRotation(InPawn->GetActorRotation(), true);
	}
	else
	{
		SetViewTarget(this);
	}

	PawnEvent.Broadcast(InPawn);
}

void AGDKPlayerController::GetPlayerViewPoint(FVector& out_Location, FRotator& out_Rotation) const
{
	if (!GetPawn())
	{
		out_Location = DeathCamera->GetComponentLocation();
		out_Rotation = DeathCamera->GetComponentRotation();
		out_Rotation.Add(0, LatestPawnYaw, 0);
	}
	else
	{
		Super::GetPlayerViewPoint(out_Location, out_Rotation);
	}
}

void AGDKPlayerController::KillCharacter(const AActor* Killer)
{
	check(GetNetMode() == NM_DedicatedServer);

	if (!HasAuthority())
	{
		return;
	}

	FString KillerName;
	int32 KillerId = -1;

	if (const AHoldable* Holdable = Cast<AHoldable>(Killer))
	{
		const AActor* Weilder = Holdable->GetOwner();


		if (const ACharacter* KillerCharacter = Cast<ACharacter>(Weilder))
		{
			if (KillerCharacter->PlayerState)
			{
				KillerName = KillerCharacter->PlayerState->GetPlayerName();
				KillerId = KillerCharacter->PlayerState->PlayerId;
			}

			if (KillerCharacter->GetController())
			{
				if (AGDKPlayerController* KillerController = Cast<AGDKPlayerController>(KillerCharacter->GetController()))
				{
					if (ACharacter* VictimCharacter = Cast<ACharacter>(GetPawn()))
					{
						APlayerState* VictimState = VictimCharacter->PlayerState;
						KillerController->InformOfKill(VictimState->GetPlayerName(), VictimState->PlayerId);
					}
				}
			}
		}
	}

	InformOfDeath(KillerName, KillerId);

	if (UScorePublisher* ScorePublisher = Cast<UScorePublisher>(GetWorld()->GetGameState()->GetComponentByClass(UScorePublisher::StaticClass())))
	{
		ScorePublisher->PublishKill(KillerId, PlayerState->PlayerId);
	}

	UnPossess();
}

void AGDKPlayerController::InformOfKill_Implementation(const FString& VictimName, int32 VictimId)
{
	KillNotification.Broadcast(VictimName, VictimId);
}
void AGDKPlayerController::InformOfDeath_Implementation(const FString& KillerName, int32 KillerId)
{
	KilledNotification.Broadcast(KillerName, KillerId);

}

void AGDKPlayerController::SetUIMode(bool bIsUIMode, bool bAllowMovement)
{
	bShowMouseCursor = bIsUIMode;
	ResetIgnoreLookInput();
	SetIgnoreLookInput(bIsUIMode);
	ResetIgnoreMoveInput();
	SetIgnoreMoveInput(bIsUIMode && !bAllowMovement);
	SetIgnoreActionInput(bIsUIMode);

	if (bIsUIMode)
	{
		SetInputMode(FInputModeGameAndUI());
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}

	if (GetPawn())
	{
		if (UEquippedComponent* EquippedComponent = Cast<UEquippedComponent>(GetPawn()->GetComponentByClass(UEquippedComponent::StaticClass())))
		{
			EquippedComponent->BlockUsing(bIsUIMode);
		}
	}
}

void AGDKPlayerController::TryJoinGame(const FString& NewPlayerName, const FGDKMetaData MetaData)
{
	check(GetNetMode() != NM_DedicatedServer);
	ServerTryJoinGame(
		NewPlayerName.IsEmpty() ? TEXT("Unknown") : NewPlayerName,
		MetaData);

}

void AGDKPlayerController::ServerTryJoinGame_Implementation(const FString& NewPlayerName, const FGDKMetaData MetaData)
{

	if (USpawnRequestPublisher* Spawner = Cast<USpawnRequestPublisher>(GetWorld()->GetGameState()->GetComponentByClass(USpawnRequestPublisher::StaticClass())))
	{
		Spawner->RequestSpawn(this);
		return;
	}
}

bool AGDKPlayerController::ServerTryJoinGame_Validate(const FString& NewPlayerName, const FGDKMetaData MetaData)
{
	return true;
}

void AGDKPlayerController::RequestRespawn()
{
	check(GetNetMode() == NM_Client);

	RespawnCharacter();
}

void AGDKPlayerController::RespawnCharacter_Implementation()
{
	if (USpawnRequestPublisher* Spawner = Cast<USpawnRequestPublisher>(GetWorld()->GetGameState()->GetComponentByClass(USpawnRequestPublisher::StaticClass())))
	{
		Spawner->RequestSpawn(this);
		return;
	}
}

bool AGDKPlayerController::RespawnCharacter_Validate()
{
	return true;
}

