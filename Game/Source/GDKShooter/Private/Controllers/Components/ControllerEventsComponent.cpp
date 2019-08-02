// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ControllerEventsComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"


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
			ClientInformOfDeath(KillerPlayerState->GetPlayerName(), KillerPlayerState->PlayerId);
		}
		else if(Killer->GetPawn() != nullptr)
		{
			ClientInformOfDeath(Killer->GetPawn()->GetHumanReadableName(),-1);
		}
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
			ClientInformOfKill(VictimPlayerState->GetPlayerName(), VictimPlayerState->PlayerId);
		}
		else if (Victim->GetPawn() != nullptr)
		{
			ClientInformOfKill(Victim->GetPawn()->GetHumanReadableName(), -1);
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
