// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Characters/Components/ShootingComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GDKLogging.h"

UShootingComponent::UShootingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaxRange = 50000.0f;
}

void UShootingComponent::BeginPlay()
{
	Super::BeginPlay();

	TArray<UActorComponent*> AllComponents;
	GetOwner()->GetComponents<UActorComponent>(AllComponents);
	bool bHasTraceProvider = false;

	for (int i = 0; i < AllComponents.Num(); i++)
	{
		if (AllComponents[i]->GetClass()->ImplementsInterface(UTraceProvider::StaticClass()))
		{
			TraceProvider.SetObject(AllComponents[i]);
			TraceProvider.SetInterface(Cast<ITraceProvider>(AllComponents[i]));
			bHasTraceProvider = true;
		}
	}

	if (!bHasTraceProvider)
	{
		UE_LOG(LogGDK, Error, TEXT("Shooting Component on %s exists without a Trace Providing component, please add an Actor Component that implements ITraceProvider."),
			*GetPathNameSafe(GetOwner()));
	}
}


FInstantHitInfo UShootingComponent::DoLineTrace(FVector Direction, AActor* ActorToIgnore)
{
	FInstantHitInfo OutHitInfo;
	
	FCollisionQueryParams TraceParams;
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;
	if (ActorToIgnore != nullptr)
	{
		TraceParams.AddIgnoredActor(ActorToIgnore);
		AActor* ActorToIgnoresOwner = ActorToIgnore->GetOwner();
		if (ActorToIgnoresOwner != nullptr)
		{
			TraceParams.AddIgnoredActor(ActorToIgnoresOwner);
		}
	}

	FHitResult HitResult(ForceInit);
	FVector TraceStart = GetLineTraceStart();
	FVector TraceEnd = TraceStart + Direction * MaxRange;

	bool bDidHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		TraceChannel,
		TraceParams);

	if (!bDidHit)
	{
		OutHitInfo.Location = TraceEnd;
		return OutHitInfo;
	}

	OutHitInfo.Location = HitResult.ImpactPoint;
	OutHitInfo.HitActor = HitResult.GetActor();

	OutHitInfo.bDidHit = true;
	return OutHitInfo;
}
