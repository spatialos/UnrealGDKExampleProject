// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LobbyTimerComponent.h"
#include "GDKLogging.h"
#include "GameFramework/Actor.h"
#include "Misc/CommandLine.h"

void ULobbyTimerComponent::BeginPlay()
{
#if !UE_BUILD_SHIPPING
	// Developer cheat so you don't have to wait for a long time before entering a match.
	const TCHAR* CommandLine = FCommandLine::Get();
	FParse::Value(CommandLine, TEXT("lobbytime"), DefaultTimerDuration);
	FParse::Value(CommandLine, TEXT("lobbyminplayers"), MinimumPlayersToStartCountdown);
#endif
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

