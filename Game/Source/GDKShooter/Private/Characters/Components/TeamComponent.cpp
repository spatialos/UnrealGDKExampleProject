// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TeamComponent.h"

#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"


UTeamComponent::UTeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTeamComponent::BeginPlay()
{
	Super::BeginPlay();

	TeamMode = Cast<UTeamModeComponent>(GetWorld()->GetAuthGameMode()->GetComponentByClass(UTeamModeComponent::StaticClass()));
}

bool UTeamComponent::CanDamageActor(AActor* OtherActor)
{
	if (!TeamMode || TeamMode->bAllowFriendlyFire)
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
