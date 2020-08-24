// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Controllers/Components/ControllerEventsComponent.h"

#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "Runtime/Launch/Resources/Version.h"

UControllerEventsComponent::UControllerEventsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UControllerEventsComponent::Death_Implementation(const AController* Killer)
{
	DeathEvent.Broadcast(Killer);

	if (Killer != nullptr)
	{
		APlayerState* KillerPlayerState = Killer->PlayerState;
		if (KillerPlayerState != nullptr)
		{
#if ENGINE_MINOR_VERSION <= 24
			ClientInformOfDeath(KillerPlayerState->GetPlayerName(), KillerPlayerState->PlayerId);
#else
			ClientInformOfDeath(KillerPlayerState->GetPlayerName(), KillerPlayerState->GetPlayerId());
#endif
		}
		else
		{
			ClientInformOfDeath(TEXT("") /*PlayerName*/, -1 /*PlayerID*/);
		}
	}
	else // Offloaded NPC
	{
		ClientInformOfDeath(TEXT("") /*PlayerName*/, -1 /*PlayerID*/);
	}
}

void UControllerEventsComponent::Kill_Implementation(const AController* Victim)
{
	KillEvent.Broadcast(Victim);

	if (Victim != nullptr)
	{
		APlayerState* VictimPlayerState = Victim->PlayerState;
		if (VictimPlayerState != nullptr)
		{
#if ENGINE_MINOR_VERSION <= 24
			ClientInformOfKill(VictimPlayerState->GetPlayerName(), VictimPlayerState->PlayerId);
#else
			ClientInformOfKill(VictimPlayerState->GetPlayerName(), VictimPlayerState->GetPlayerId());
#endif
		}
	}
}

void UControllerEventsComponent::ClientInformOfKill_Implementation(const FString& VictimName, int32 VictimId)
{
	KillDetailsEvent.Broadcast(VictimName, VictimId);
}

void UControllerEventsComponent::ClientInformOfDeath_Implementation(const FString& KillerName, int32 KillerId)
{
	DeathDetailsEvent.Broadcast(KillerName, KillerId);
}
