// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Testing/TestingObject.h"
#include "GameFramework/Actor.h"
#include "NonReplicatedTestingObject.generated.h"

UCLASS(Blueprintable)
class GDKSHOOTER_API ANonReplicatedTestingObject : public ATestingObject
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANonReplicatedTestingObject(const FObjectInitializer& ObjectInitializer);
};
