// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKWidget.h"
#include "Game/GDKGameState.h"
#include "Game/GDKSessionGameState.h"
#include "Game/GDKPlayerState.h"
#include "Controllers/GDKPlayerController.h"
#include "Components/GDKMovementComponent.h"
#include "Components/HealthComponent.h"
#include "GDKLogging.h"

// Registr listeners on AGDKPlayerController and AGDKGameState
void UGDKWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (bListenersAdded)
	{
		return;
	}
	else
	{
		bListenersAdded = true;
	}

	PlayerController = GetOwningPlayer();

	if (AGDKPlayerController* GDKPC = Cast<AGDKPlayerController>(PlayerController))
	{
		GDKPC->OnPawn().AddDynamic(this, &UGDKWidget::OnPawn);

		GDKPC->OnKillNotification().AddUObject(this, &UGDKWidget::OnKill);
		GDKPC->OnKilledNotification().AddUObject(this, &UGDKWidget::OnDeath);
	}

	if (AGDKGameState* GS = GetWorld()->GetGameState<AGDKGameState>())
	{
		GS->OnScoreUpdated().AddUObject(this, &UGDKWidget::OnPlayerScoresUpdated);
		OnPlayerScoresUpdated(GS->PlayerScores());
		GS->OnPlayerCountUpdated().AddUObject(this, &UGDKWidget::OnPlayerCountUpdated);
		OnPlayerCountUpdated(GS->ConnectedPlayers);
	}

	if (AGDKSessionGameState* SGS = GetWorld()->GetGameState<AGDKSessionGameState>())
	{
		SGS->OnTimerUpdated().AddUObject(this, &UGDKWidget::OnTimerUpdated);
		OnTimerUpdated(SGS->SessionProgress, SGS->SessionTimer);
	}
}

void UGDKWidget::OnPawn(APawn* InPawn)
{
	if (!InPawn)
	{
		return;
	}

	if (UGDKMovementComponent* Movement = Cast< UGDKMovementComponent>(InPawn->GetComponentByClass(UGDKMovementComponent::StaticClass())))
	{
		Movement->OnAimingUpdated.RemoveDynamic(this, &UGDKWidget::OnAimingUpdated);
		Movement->OnAimingUpdated.AddDynamic(this, &UGDKWidget::OnAimingUpdated);
	}

	if (UHealthComponent* Health = Cast< UHealthComponent>(InPawn->GetComponentByClass(UHealthComponent::StaticClass())))
	{
		Health->HealthUpdated.RemoveDynamic(this, &UGDKWidget::OnHealthUpdated);
		Health->HealthUpdated.AddDynamic(this, &UGDKWidget::OnHealthUpdated);
		Health->ArmourUpdated.RemoveDynamic(this, &UGDKWidget::OnArmourUpdated);
		Health->ArmourUpdated.AddDynamic(this, &UGDKWidget::OnArmourUpdated);
		OnHealthUpdated(Health->GetCurrentHealth(), Health->GetMaxHealth());
		OnArmourUpdated(Health->GetCurrentArmour(), Health->GetMaxArmour());
	}
}

// Function to call to ClientTravel to the TargetMap
void UGDKWidget::LeaveGame(const FString& TargetMap)
{
	check(PlayerController);

	FURL TravelURL;
	TravelURL.Map = *TargetMap;

	PlayerController->ClientTravel(TravelURL.ToString(), TRAVEL_Absolute, false /*bSeamless*/);

}


