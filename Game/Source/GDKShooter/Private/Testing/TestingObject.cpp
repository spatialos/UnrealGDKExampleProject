// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "TestingObject.h"

// Sets default values
ATestingObject::ATestingObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CubeMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("CubeMesh"));

	this->SetRootComponent(CubeMesh);
}

// Called when the game starts or when spawned
void ATestingObject::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATestingObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

