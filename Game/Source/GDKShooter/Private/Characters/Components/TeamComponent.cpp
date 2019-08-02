// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TeamComponent.h"
#include "UnrealNetwork.h"

#include "Engine/World.h"


UTeamComponent::UTeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bReplicates = true;
}

void UTeamComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTeamComponent, TeamId);
}

bool UTeamComponent::CanDamageActor(AActor* OtherActor)
{
	if (OtherActor == nullptr)
	{
		return false;
	}

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
