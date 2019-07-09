// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TeamComponent.h"

#include "Engine/World.h"


UTeamComponent::UTeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UTeamComponent::CanDamageActor(AActor* OtherActor)
{
	if (bAllowFriendlyFire)
	{
		return true;
	}

	if (!HasTeam())
	{
		return true;
	}

	if (UTeamComponent* OtherTeamComponent = Cast<UTeamComponent>(OtherActor->GetComponentByClass(UTeamComponent::StaticClass())))
	{
		return !OtherTeamComponent->HasTeam() || OtherTeamComponent->GetTeam() != GetTeam();
	}

	return true;
}
