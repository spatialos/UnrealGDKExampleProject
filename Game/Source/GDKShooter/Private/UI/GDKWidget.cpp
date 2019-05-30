// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKWidget.h"
#include "GameFramework/GameStateBase.h"
#include "Controllers/GDKPlayerController.h"
#include "Components/GDKMovementComponent.h"
#include "Components/HealthComponent.h"
#include "Game/Components/PlayerCountingComponent.h"
#include "Game/Components/LobbyTimerComponent.h"
#include "Game/Components/MatchTimerComponent.h"
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

	if (UDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass())))
	{
		Deathmatch->ScoreEvent.AddDynamic(this, &UGDKWidget::OnPlayerScoresUpdated);
		OnPlayerScoresUpdated(Deathmatch->PlayerScores());
	}

	if (UPlayerCountingComponent* PlayerCounter = Cast<UPlayerCountingComponent>(GetWorld()->GetGameState()->GetComponentByClass(UPlayerCountingComponent::StaticClass())))
	{
		PlayerCounter->PlayerCountEvent.AddDynamic(this, &UGDKWidget::OnPlayerCountUpdated);
		OnPlayerCountUpdated(PlayerCounter->PlayerCount());
	}

	if (UMatchStateComponent* MatchState = Cast<UMatchStateComponent>(GetWorld()->GetGameState()->GetComponentByClass(UMatchStateComponent::StaticClass())))
	{
		MatchState->MatchEvent.AddDynamic(this, &UGDKWidget::OnStateUpdated);
		OnStateUpdated(MatchState->GetCurrentState());
	}

	if (UMatchTimerComponent* MatchTimer = Cast<UMatchTimerComponent>(GetWorld()->GetGameState()->GetComponentByClass(UMatchTimerComponent::StaticClass())))
	{
		MatchTimer->OnTimer.AddDynamic(this, &UGDKWidget::OnMatchTimerUpdated);
		OnMatchTimerUpdated(MatchTimer->GetTimer());
	}

	if (ULobbyTimerComponent* LobbyTimer = Cast<ULobbyTimerComponent>(GetWorld()->GetGameState()->GetComponentByClass(ULobbyTimerComponent::StaticClass())))
	{
		LobbyTimer->OnTimer.AddDynamic(this, &UGDKWidget::OnLobbyTimerUpdated);
		OnLobbyTimerUpdated(LobbyTimer->GetTimer());
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
		Movement->OnAimingUpdated.AddUniqueDynamic(this, &UGDKWidget::OnAimingUpdated);
	}

	if (UHealthComponent* Health = Cast< UHealthComponent>(InPawn->GetComponentByClass(UHealthComponent::StaticClass())))
	{
		Health->HealthUpdated.AddUniqueDynamic(this, &UGDKWidget::OnHealthUpdated);
		Health->ArmourUpdated.AddUniqueDynamic(this, &UGDKWidget::OnArmourUpdated);
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


