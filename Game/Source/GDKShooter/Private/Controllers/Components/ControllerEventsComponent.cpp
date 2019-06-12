// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ControllerEventsComponent.h"
#include "GDKLogging.h"


UControllerEventsComponent::UControllerEventsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UControllerEventsComponent::KilledBy(const AActor* Killer)
{
	KillEvent.Broadcast(Killer);
}

