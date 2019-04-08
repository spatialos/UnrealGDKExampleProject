// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKWidget.h"
#include "Game/GDKGameState.h"
#include "Game/GDKSessionGameState.h"
#include "Game/GDKPlayerState.h"
#include "Controllers/GDKPlayerController.h"
#include "GDKLogging.h"

// Registr listeners on AGDKPlayerController and AGDKGameState
void UGDKWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerController = GetOwningPlayer();
	if (PlayerController->PlayerState)
	{
		OwnId = PlayerController->PlayerState->PlayerId;
	}

	if (AGDKPlayerController* GDKPC = Cast<AGDKPlayerController>(PlayerController))
	{
		GDKPC->OnHealthUpdated().AddUObject(this, &UGDKWidget::OnHealthUpdated);
		GDKPC->OnArmourUpdated().AddUObject(this, &UGDKWidget::OnArmourUpdated);
		GDKPC->OnKillNotification().AddUObject(this, &UGDKWidget::OnKill);
		GDKPC->OnKilledNotification().AddUObject(this, &UGDKWidget::OnDeath);
		GDKPC->OnAimingUpdated().AddUObject(this, &UGDKWidget::OnAimingUpdated);
		GDKPC->OnWeaponChanged().AddUObject(this, &UGDKWidget::OnWeaponChanged);
		GDKPC->OnShot().AddUObject(this, &UGDKWidget::OnShot);
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

// Function to call to ClientTravel to the TargetMap
void UGDKWidget::LeaveGame(const FString& TargetMap)
{
	check(PlayerController);

	FURL TravelURL;
	TravelURL.Map = *TargetMap;

	PlayerController->ClientTravel(TravelURL.ToString(), TRAVEL_Absolute, false /*bSeamless*/);

}


