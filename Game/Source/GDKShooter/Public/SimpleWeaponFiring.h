// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimpleWeaponFiring.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API USimpleWeaponFiring : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USimpleWeaponFiring();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerFire();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
