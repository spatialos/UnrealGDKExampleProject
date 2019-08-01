// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LobbyTimerComponent.h"
#include "GDKLogging.h"
#include "GameFramework/Actor.h"

void ULobbyTimerComponent::BeginPlay()
{
	bAutoStart = (MinimumPlayersToStartCountdown == 0);
	Super::BeginPlay();
}

void ULobbyTimerComponent::ServerInformOfPlayerCount_Implementation(int32 PlayerCount)
{
	if (bHasTimerFinished || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!bIsTimerRunning && PlayerCount >= MinimumPlayersToStartCountdown)
	{
		StartTimer();
	}
	else if (bIsTimerRunning && PlayerCount < MinimumPlayersToStartCountdown)
	{
		StopTimer();
		SetTimer(DefaultTimerDuration);
	}
}

