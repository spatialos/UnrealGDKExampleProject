// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "Testing/NonReplicatedTestingObject.h"

#include "Net/UnrealNetwork.h"
#include "TestingConstants.h"
#include "Characters/GDKCharacter.h"

// Sets default values
ANonReplicatedTestingObject::ANonReplicatedTestingObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = false;
}
