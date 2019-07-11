// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Controllers/GDKPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/ControllerEventsComponent.h"
#include "Components/EquippedComponent.h"
#include "Components/HealthComponent.h"
#include "Components/MetaDataComponent.h"
#include "Connection/SpatialWorkerConnection.h"
#include "Game/Components/ScorePublisher.h"
#include "Game/Components/SpawnRequestPublisher.h"
#include "Game/Components/PlayerPublisher.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "SpatialNetDriver.h"
#include "UnrealNetwork.h"
#include "Weapons/Holdable.h"
#include "Weapons/Projectile.h"
#include "Weapons/Weapon.h"


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
	OnNewPawn(InPawn);
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

void AGDKPlayerController::SetUIMode(bool bIsUIMode)
{
	bShowMouseCursor = bIsUIMode;
	ResetIgnoreLookInput();
	SetIgnoreLookInput(bIsUIMode);
	ResetIgnoreMoveInput();
	SetIgnoreMoveInput(bIsUIMode);

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

void AGDKPlayerController::ServerTryJoinGame_Implementation()
{

	if (USpawnRequestPublisher* Spawner = Cast<USpawnRequestPublisher>(GetWorld()->GetGameState()->GetComponentByClass(USpawnRequestPublisher::StaticClass())))
	{
		Spawner->RequestSpawn(this);
		return;
	}
}

bool AGDKPlayerController::ServerTryJoinGame_Validate()
{
	return true;
}

void AGDKPlayerController::ServerRequestName_Implementation(const FString& NewPlayerName)
{
	if (PlayerState)
	{
		PlayerState->SetPlayerName(NewPlayerName);
	}
}

bool AGDKPlayerController::ServerRequestName_Validate(const FString& NewPlayerName)
{
	return true;
}

void AGDKPlayerController::ServerRequestMetaData_Implementation(const FGDKMetaData NewMetaData)
{
	if (UMetaDataComponent* MetaData = Cast<UMetaDataComponent>(PlayerState->GetComponentByClass(UMetaDataComponent::StaticClass())))
	{
		MetaData->SetMetaData(NewMetaData);
	}
}

bool AGDKPlayerController::ServerRequestMetaData_Validate(const FGDKMetaData NewMetaData)
{
	return true;
}

void AGDKPlayerController::ServerRespawnCharacter_Implementation()
{
	if (USpawnRequestPublisher* Spawner = Cast<USpawnRequestPublisher>(GetWorld()->GetGameState()->GetComponentByClass(USpawnRequestPublisher::StaticClass())))
	{
		Spawner->RequestSpawn(this);
		return;
	}
}

bool AGDKPlayerController::ServerRespawnCharacter_Validate()
{
	return true;
}

