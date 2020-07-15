// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Characters/Components/TeamComponent.h"
#include "Net/UnrealNetwork.h"

#include "Engine/World.h"


UTeamComponent::UTeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UTeamComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTeamComponent, TeamId);
}

bool UTeamComponent::CanDamageActor(AActor* OtherActor)
{
	if (!IsValid(OtherActor))
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
