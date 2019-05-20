// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/ITraceProvider.h"
#include "ShootingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShotEvent, AWeapon*, Weapon);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UShootingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UShootingComponent();

	void BeginPlay();

	UPROPERTY(BlueprintAssignable)
		FShotEvent ShotEvent;

	void FireShot(AWeapon* Weapon) { ShotEvent.Broadcast(Weapon); }

	UFUNCTION(BlueprintCallable)
	FVector GetLineTraceStart()
	{
		UObject* TraceProviderObject = TraceProvider.GetObject();
		return ITraceProvider::Execute_GetLineTraceStart(TraceProvider.GetObject());
	}

	UFUNCTION(BlueprintCallable)
	FVector GetLineTraceDirection()
	{
		UObject* TraceProviderObject = TraceProvider.GetObject();
		return ITraceProvider::Execute_GetLineTraceDirection(TraceProvider.GetObject());
	}
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
		TScriptInterface<ITraceProvider> TraceProvider;

};
