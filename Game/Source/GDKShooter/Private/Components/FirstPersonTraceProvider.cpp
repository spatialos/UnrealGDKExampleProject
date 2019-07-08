// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "FirstPersonTraceProvider.h"
#include "GDKLogging.h"

UFirstPersonTraceProvider::UFirstPersonTraceProvider()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFirstPersonTraceProvider::BeginPlay()
{
	Super::BeginPlay();
	FirstPersonCamera = Cast<UCameraComponent>(GetOwner()->GetComponentByClass(UCameraComponent::StaticClass()));

	if (!FirstPersonCamera)
	{
		UE_LOG(LogGDK, Error, TEXT("FirstPersonTraceProvider Exists without a Camera Component."));
	}
}

FVector UFirstPersonTraceProvider::GetLineTraceStart_Implementation() const
{
	return FirstPersonCamera->GetComponentLocation();
}


FVector UFirstPersonTraceProvider::GetLineTraceDirection_Implementation() const
{
	return FirstPersonCamera->GetForwardVector();
}
