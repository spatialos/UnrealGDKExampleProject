// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "SimpleWeaponFiring.h"
#include "Camera/CameraComponent.h"

// Sets default values for this component's properties
USimpleWeaponFiring::USimpleWeaponFiring()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USimpleWeaponFiring::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void USimpleWeaponFiring::Fire()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner->Role < ROLE_Authority)
	{
		//This part only runs on clients. The client does not make line tracing or hit detection, it only plays the muzzle FX and calls the ServerFire
		//function that is run on the server and simply calls Fire() again
		UE_LOG(LogTemp, Warning, TEXT("***** CLIENT PEW *****"));
		ServerFire();
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("***** SERVER PEW *****"));

	FHitResult OutHit;
	//FVector Start = 

	//FVector ForwardVector = MyOwner->GetComponentsByClass<UCameraComponent>();
}

void USimpleWeaponFiring::ServerFire_Implementation()
{
	Fire();
}

// Called every frame
void USimpleWeaponFiring::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

