// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Testing/TestingObject.h"

#include "TestingComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UTestingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTestingComponent();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MarkActorAsCheckedOutByClient(ATestingObject* TestingObject);
};
