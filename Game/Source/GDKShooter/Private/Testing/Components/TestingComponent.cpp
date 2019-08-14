// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestingComponent.h"

UTestingComponent::UTestingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bReplicates = true;
}

void UTestingComponent::Server_MarkActorAsCheckedOutByClient_Implementation(ATestingObject* TestingObject)
{
	TestingObject->HasBeenCheckedOutByClient = true;
}

bool UTestingComponent::Server_MarkActorAsCheckedOutByClient_Validate(ATestingObject* TestingObject)
{
	return true;
}
