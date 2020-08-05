// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Characters/GDKCharacter.h"
#include "GDKSimulatedCharacter.generated.h"

UCLASS()
class GDKSHOOTER_API AGDKSimulatedCharacter : public AGDKCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGDKSimulatedCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
