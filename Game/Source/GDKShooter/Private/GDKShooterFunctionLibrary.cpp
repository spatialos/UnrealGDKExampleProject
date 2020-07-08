// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKShooterFunctionLibrary.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"


void UGDKShooterFunctionLibrary::SetGenericTeamId(AActor* Actor, FGenericTeamId NewTeamId)
{
	IGenericTeamAgentInterface* TeamAgentActor = Cast<IGenericTeamAgentInterface>(Actor);
	IGenericTeamAgentInterface* TeamAgentController = nullptr;
	if (APawn* AsPawn = Cast<APawn>(Actor))
	{
		TeamAgentController = Cast<IGenericTeamAgentInterface>(AsPawn->GetController());
	}

	if (TeamAgentActor != nullptr)
	{
		TeamAgentActor->SetGenericTeamId(NewTeamId);
	}

	if (TeamAgentController != nullptr)
	{
		TeamAgentController->SetGenericTeamId(NewTeamId);
	}

	if (UAIPerceptionStimuliSourceComponent* PerceptionStimuli = Actor->FindComponentByClass<UAIPerceptionStimuliSourceComponent>())
	{
		PerceptionStimuli->UnregisterFromPerceptionSystem();
		PerceptionStimuli->RegisterWithPerceptionSystem();
	}
}

FGenericTeamId UGDKShooterFunctionLibrary::GetGenericTeamId(AActor* Actor)
{
	IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(Actor);

	if (TeamAgent == nullptr)
	{
		if (APawn* AsPawn = Cast<APawn>(Actor))
		{
			TeamAgent = Cast<IGenericTeamAgentInterface>(AsPawn->GetController());
		}
	}

	if (TeamAgent != nullptr)
	{
		return TeamAgent->GetGenericTeamId();
	}

	return FGenericTeamId::NoTeam;
}
