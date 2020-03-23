// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Game/Components/PlayerCountingComponent.h"
#include "Net/UnrealNetwork.h"
#include "GDKLogging.h"


UPlayerCountingComponent::UPlayerCountingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPlayerCountingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerCountingComponent, ConnectedPlayerCount);
}

void UPlayerCountingComponent::PlayerEvent(APlayerState* PlayerState, EPlayerProgress Progress)
{
	if (Progress == EPlayerProgress::Connected)
	{
		ConnectedPlayerCount++;
	}
	else if (Progress == EPlayerProgress::Disconnected)
	{
		ConnectedPlayerCount--;
	}
	PlayerCountEvent.Broadcast(ConnectedPlayerCount);
}

void UPlayerCountingComponent::OnRep_ConnectedPlayerCount() 
{
	PlayerCountEvent.Broadcast(ConnectedPlayerCount);
}
