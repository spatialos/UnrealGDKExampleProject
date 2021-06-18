// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Game/Components/MatchStateComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

UMatchStateComponent::UMatchStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMatchStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMatchStateComponent, CurrentState);
}

void UMatchStateComponent::SetMatchState(EMatchState NewState) 
{ 
	if (!GetOwner()->HasAuthority() || CurrentState == EMatchState::NetworkFailure)
	{
		return;
	}

	CurrentState = NewState;
	OnRep_State();
}
