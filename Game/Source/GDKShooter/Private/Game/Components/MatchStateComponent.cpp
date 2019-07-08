// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "MatchStateComponent.h"
#include "UnrealNetwork.h"
#include "GameFramework/Actor.h"

UMatchStateComponent::UMatchStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bReplicates = true;
}

void UMatchStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMatchStateComponent, CurrentState);
}

void UMatchStateComponent::SetMatchState(EMatchState NewState) 
{ 
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	CurrentState = NewState;
	OnRep_State();
}
