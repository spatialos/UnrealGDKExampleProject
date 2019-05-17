// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ShootingComponent.h"
#include "GDKLogging.h"
#include "Characters/Core/GDKEquippedCharacter.h"

UShootingComponent::UShootingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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
		UE_LOG(LogGDK, Error, TEXT("Shooting Component Exists without a Trace Providing component, please add an Actor Component that implements ITraceProvider."));
	}
}

