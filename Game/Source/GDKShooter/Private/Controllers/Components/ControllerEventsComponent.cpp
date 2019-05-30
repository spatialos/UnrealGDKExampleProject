// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ControllerEventsComponent.h"
#include "GDKLogging.h"


UControllerEventsComponent::UControllerEventsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UControllerEventsComponent::KilledBy(const AActor* Killer)
{
	UE_LOG(LogGDK, Error, TEXT("%s Killed by %s"), *GetOwner()->GetName(), *Killer->GetName());
	KillEvent.Broadcast(Killer);
}

