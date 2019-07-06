// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Controllers/GDKPlayerController.h"

#include "Characters/Core/GDKShooterCharacter.h"
#include "Game/GDKGameState.h"
#include "Game/GDKSessionGameState.h"
#include "Game/GDKPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "GDKLogging.h"
#include "UnrealNetwork.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

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

	if (AGDKSessionGameState* GameState = Cast<AGDKSessionGameState>(GetWorld()->GetGameState()))
	{
		GameState->OnTimerUpdated().AddUObject(this, &AGDKPlayerController::TimerUpdated);
	}
	SetUIMode(true, false);
}

void AGDKPlayerController::Tick(float DeltaTime)
{
	if (GetPawn())
	{
		LatestPawnYaw = GetPawn()->GetActorRotation().Yaw;
	}
}

void AGDKPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void AGDKPlayerController::UpdateHealthUI(int32 NewHealth, int32 MaxHealth)
{
	HealthChangedEvent.Broadcast(NewHealth, MaxHealth);
}

void AGDKPlayerController::UpdateArmourUI(int32 NewArmour, int32 MaxArmour)
{
	ArmourChangedEvent.Broadcast(NewArmour, MaxArmour);
}

void AGDKPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (GetNetMode() == NM_Client)
	{
		AGDKCharacter* GDKCharacter = Cast<AGDKCharacter>(InPawn);
		if (GDKCharacter != nullptr)
		{
			SetCharacterState(EGDKCharacterState::Alive);

			UpdateHealthUI(GDKCharacter->GetCurrentHealth(), GDKCharacter->GetMaxHealth());
			UpdateArmourUI(GDKCharacter->GetCurrentArmour(), GDKCharacter->GetMaxArmour());

			// Make the new pawn's camera this controller's camera.
			SetViewTarget(InPawn);

			this->ClientSetRotation(InPawn->GetActorRotation(), true);
		}
		else
		{
			SetViewTarget(this);
		}

		AGDKShooterCharacter* ShooterCharacter = Cast<AGDKShooterCharacter>(InPawn);
		if (ShooterCharacter != nullptr)
		{
			FAimingStateChanged AimingCallback;
			AimingCallback.BindUObject(this, &AGDKPlayerController::AimingChanged);
			ShooterCharacter->AddAimingListener(AimingCallback);
			AimingChanged(ShooterCharacter->IsAiming(), 1);

			FWeaponShotDelegate ShotCallback;
			ShotCallback.BindUObject(this, &AGDKPlayerController::OnCharacterShot);
			ShooterCharacter->AddShotListener(ShotCallback);

			FWeaponChanged WeaponCallback;
			WeaponCallback.BindUObject(this, &AGDKPlayerController::WeaponChanged);
			ShooterCharacter->AddWeaponListener(WeaponCallback);

		}
	}
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

void AGDKPlayerController::OnCharacterShot(AWeapon* Weapon, bool Hit)
{
	if (Weapon)
	{
		OnShot(Weapon, Hit);
	}
	ShotEvent.Broadcast(Weapon, Hit);
}

void AGDKPlayerController::AimingChanged(bool bIsAiming, float AimRotationSpeed)
{
	AimingChangedEvent.Broadcast(bIsAiming);
	OnAimingChanged(bIsAiming, AimRotationSpeed);
}

void AGDKPlayerController::WeaponChanged(AWeapon* Weapon)
{
	WeaponNotification.Broadcast(Weapon);
}

void AGDKPlayerController::KillCharacter(const AGDKCharacter* Killer)
{
	check(GetNetMode() == NM_DedicatedServer);

	if (!HasAuthority())
	{
		return;
	}

	FString KillerName;

	if (Killer)
	{
		KillerName = Killer->GetPlayerName();
		InformOfDeath(KillerName, Killer->PlayerId);

		if (Killer->GetController())
		{
			if (AGDKPlayerController* KC = Cast<AGDKPlayerController>(Killer->GetController()))
			{
				if (AGDKCharacter* PC = Cast<AGDKCharacter>(GetPawn()))
				{
					KC->InformOfKill(PC->GetPlayerName(), PC->PlayerId);
				}
			}
		}
	}

	if (AGDKGameState* GM = Cast<AGDKGameState>(GetWorld()->GetGameState()))
	{
		if (AGDKPlayerState* PS = Cast<AGDKPlayerState>(PlayerState))
			GM->AddDeath(Killer->PlayerId, PS->PlayerId);
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
	SetCharacterState(EGDKCharacterState::Dead);
	HealthChangedEvent.Broadcast(0, 100);
	ArmourChangedEvent.Broadcast(0, 100);

}

void AGDKPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("ShowScoreboard", IE_Pressed, this, &AGDKPlayerController::ShowScoreboard);
	InputComponent->BindAction("ShowScoreboard", IE_Released, this, &AGDKPlayerController::HideScoreboard);
	InputComponent->BindAction("ShowMenu", IE_Pressed, this, &AGDKPlayerController::ToggleMenu);
}

void AGDKPlayerController::TimerUpdated(EGDKSessionProgress SessionProgress, int SessionTimer)
{
	if (SessionProgress == EGDKSessionProgress::Results && !bGameFinished)
	{
		bGameFinished = true;
		SetControllerState(EGDKControllerState::Finished);
	}
	if (SessionProgress == EGDKSessionProgress::Finished)
	{
		FURL TravelURL;
		TravelURL.Map = TEXT("Deployments");
		ClientTravel(TravelURL.ToString(), TRAVEL_Absolute, false /*bSeamless*/);
	}
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

		AGDKShooterCharacter* ShooterCharacter = Cast<AGDKShooterCharacter>(GetPawn());
		if (ShooterCharacter != nullptr)
		{
			ShooterCharacter->StopFire();
		}
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}
}

void AGDKPlayerController::TryJoinGame(const FString& NewPlayerName, const FGDKMetaData MetaData)
{
	check(GetNetMode() != NM_DedicatedServer);
	SetControllerState(EGDKControllerState::PendingCharacter);
	bHasRequetsedPlayer = true;
	ServerTryJoinGame(
		NewPlayerName.IsEmpty() ? TEXT("Unknown") : NewPlayerName,
		MetaData);
}

void AGDKPlayerController::ChooseNewSpawnPoint()
{
	AActor* const NewStartSpot = GetWorld()->GetAuthGameMode()->ChoosePlayerStart(this);
	if (NewStartSpot != nullptr)
	{
		// Set the player controller / camera in this new location
		FRotator InitialControllerRot = NewStartSpot->GetActorRotation();
		InitialControllerRot.Roll = 0.f;
		SetInitialLocationAndRotation(NewStartSpot->GetActorLocation(), InitialControllerRot);
		StartSpot = NewStartSpot;
	}
}

void AGDKPlayerController::ServerTryJoinGame_Implementation(const FString& NewPlayerName, const FGDKMetaData MetaData)
{
	bool bJoinWasSuccessful = true;

	// Validate player name
	if (NewPlayerName.IsEmpty())
	{
		bJoinWasSuccessful = false;

		UE_LOG(LogGDK, Error, TEXT("%s PlayerController: Player attempted to join with empty name."), *this->GetName());
	}

	// Validate PlayerState
	if (PlayerState == nullptr
		|| !PlayerState->IsA(AGDKPlayerState::StaticClass()))
	{
		bJoinWasSuccessful = false;

		UE_LOG(LogGDK, Error, TEXT("%s PlayerController: Invalid PlayerState pointer (%s)"), *this->GetName(), PlayerState == nullptr ? TEXT("nullptr") : *PlayerState->GetName());
	}

	// Validate the join request
	if (bHasBeenGrantedPlayer)
	{
		bJoinWasSuccessful = false;

		UE_LOG(LogGDK, Error, TEXT("%s PlayerController: Already submitted Join request.  Client attempting to join session multiple times."), *this->GetName());
	}

	// Inform Client as to whether or not join was accepted
	ClientJoinResults(bJoinWasSuccessful);

	if (bJoinWasSuccessful)
	{
		bHasBeenGrantedPlayer = true;

		// Set the player-selected values
		PlayerState->SetPlayerName(NewPlayerName);
		Cast<AGDKPlayerState>(PlayerState)->SetMetaData(MetaData);

		// Spawn the Pawn
		RespawnCharacter();

		// Add the player to the game's scoreboard.
		if (AGDKGameState* GS = GetWorld()->GetGameState<AGDKGameState>())
		{
			GS->AddPlayer(PlayerState->PlayerId, NewPlayerName);
		}
		else
		{
			UE_LOG(LogGDK, Error, TEXT("%s: failed to add player because GameMode didn't exist"),
				*GDKLogging::LogPrefix(this));
		}
	}

}

bool AGDKPlayerController::ServerTryJoinGame_Validate(const FString& NewPlayerName, const FGDKMetaData MetaData)
{
	return true;
}

void AGDKPlayerController::ClientJoinResults_Implementation(const bool bJoinSucceeded)
{
	if (bJoinSucceeded)
	{
		bHasBeenGrantedPlayer = true;
		SetControllerState(EGDKControllerState::InProgress);
	}
	else
	{
		bHasRequetsedPlayer = false;
		SetControllerState(EGDKControllerState::PreCharacter);
	}

}

void AGDKPlayerController::RequestRespawn()
{
	check(GetNetMode() == NM_Client);

	if (CurrentCharacterState != EGDKCharacterState::Dead)
	{
		return;
	}

	SetCharacterState(EGDKCharacterState::PendingRespawn);

	RespawnCharacter();
}

bool AGDKPlayerController::RespawnCharacter_Validate()
{
	return true;
}

void AGDKPlayerController::RespawnCharacter_Implementation()
{
	check(GetNetMode() == NM_DedicatedServer);

	if (bGameFinished)
	{
		return;
	}

	if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
	{
		APawn* NewPawn = nullptr; 
		
		ChooseNewSpawnPoint();

		check(StartSpot.IsValid());

		NewPawn = GameMode->SpawnDefaultPawnFor(this, StartSpot.Get());

		Possess(NewPawn);

		AGDKCharacter* NewCharacter = Cast<AGDKCharacter>(NewPawn);
		if (NewCharacter != nullptr)
		{
			AGDKPlayerState* GDKPlayerState = Cast<AGDKPlayerState>(PlayerState);
			if (GDKPlayerState)
			{
				NewCharacter->SetMetaData(GDKPlayerState->GetMetaData());
				NewCharacter->PlayerId = GDKPlayerState->PlayerId;
			}
			else 
			{
				UE_LOG(LogGDK, Error, TEXT("%d: Created a player without a PlayerState"), *this->GetName());
			}
		}
	}
}

void AGDKPlayerController::SetUIMode()
{
	bool bInMenu = CurrentControllerState != EGDKControllerState::InProgress || CurrentMenu != EGDKMenu::None || CurrentCharacterState != EGDKCharacterState::Alive;
	SetUIMode(bInMenu, !bInMenu);
}

void AGDKPlayerController::SetControllerState(EGDKControllerState NewState)
{
	CurrentControllerState = NewState;
	OnControllerState.Broadcast(CurrentControllerState);
	//There is currently no state transition where we should be keeping a menu window open
	SetUIMode();
}

void AGDKPlayerController::SetCharacterState(EGDKCharacterState NewState)
{
	CurrentCharacterState = NewState;
	OnCharacterState.Broadcast(CurrentCharacterState);
	SetUIMode();
}

void AGDKPlayerController::SetMenu(EGDKMenu NewMenu)
{
	CurrentMenu = NewMenu;
	OnMenuChanged.Broadcast(CurrentMenu);
	SetUIMode();
}

void AGDKPlayerController::ShowScoreboard()
{
	if (CurrentControllerState == EGDKControllerState::InProgress)
	{
		SetMenu(EGDKMenu::Scores);
	}
}
void AGDKPlayerController::HideScoreboard()
{
	if (CurrentMenu == EGDKMenu::Scores)
	{
		SetMenu(EGDKMenu::None);
	}
}
void AGDKPlayerController::ToggleMenu()
{
	if (CurrentControllerState == EGDKControllerState::InProgress)
	{
		if (CurrentMenu != EGDKMenu::Menu)
		{
			SetMenu(EGDKMenu::Menu);
		}
		else
		{
			SetMenu(EGDKMenu::None);
		}
	}
}
