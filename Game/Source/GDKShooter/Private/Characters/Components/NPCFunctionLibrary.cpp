// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "NPCFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Runtime/AIModule/Classes/Perception/AISightTargetInterface.h"

bool UNPCFunctionLibrary::TargetLocation(const APawn* SelfAsPawn, const AActor* Target, FVector& Location)
{
	if (Target == nullptr)
	{
		return false;
	}

	if (const IAISightTargetInterface* Sight = Cast<IAISightTargetInterface>(Target))
	{
		int32 LoSChecks;
		float Strength;
		FVector ViewLocation;
		FRotator ViewRotation;
		SelfAsPawn->GetActorEyesViewPoint(ViewLocation, ViewRotation);

		if (Sight->CanBeSeenFrom(ViewLocation, Location, LoSChecks, Strength, SelfAsPawn))
		{
			return Strength > 0.f;
		}
		else 
		{
			return false;
		}
	}
	else
	{
		Location = Target->GetActorLocation();
		return true;
	}
}
