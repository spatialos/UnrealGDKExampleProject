// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "Characters/GDKSimulatedCharacter.h"

// Sets default values
AGDKSimulatedCharacter::AGDKSimulatedCharacter(const FObjectInitializer& ObjectInitializer)
	:
	AGDKCharacter(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	// PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGDKSimulatedCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGDKSimulatedCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

