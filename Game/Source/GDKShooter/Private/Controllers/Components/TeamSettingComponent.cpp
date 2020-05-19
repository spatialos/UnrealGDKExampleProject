// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Controllers/Components/TeamSettingComponent.h"
#include "GDKLogging.h"
#include "AIController.h"

UTeamSettingComponent::UTeamSettingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTeamSettingComponent::UpdateTeam(FGenericTeamId InTeamId)
{
	TeamId = InTeamId;
	if (AAIController* OwningController = Cast<AAIController>(GetOwner()))
	{
		OwningController->SetGenericTeamId(TeamId);
	}
	else
	{
		UE_LOG(LogGDK, Error, TEXT("TeamSettingComponent is designed to work with AIControllers, will not work with %s"), *GetOwner()->GetName());
	}
}

void UTeamSettingComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateTeam(TeamId);
}

