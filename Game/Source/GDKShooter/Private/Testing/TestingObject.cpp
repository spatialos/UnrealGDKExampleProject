// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "TestingObject.h"

#include "Net/UnrealNetwork.h"
#include "TestingConstants.h"

// Sets default values
ATestingObject::ATestingObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CubeMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("CubeMesh"));
	this->SetRootComponent(CubeMesh);

	TestProperty = TestingConstants::DEFAULT_TESTING_PROPERTY_VALUE;
}

void ATestingObject::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	DOREPLIFETIME(ATestingObject, TestProperty);
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

void ATestingObject::SetTestProperty(const FString& NewTestPropertyValue)
{
	TestProperty = NewTestPropertyValue;
}

