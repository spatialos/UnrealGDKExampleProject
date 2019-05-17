// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/ITraceProvider.h"
#include "Camera/CameraComponent.h"

#include "FirstPersonTraceProvider.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UFirstPersonTraceProvider : public UActorComponent, public ITraceProvider
{
	GENERATED_BODY()

public:	
	UFirstPersonTraceProvider();

	virtual void BeginPlay();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Trace Provider")
		FVector GetLineTraceStart() const;
	virtual FVector GetLineTraceStart_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Trace Provider")
		FVector GetLineTraceDirection() const;
	virtual FVector GetLineTraceDirection_Implementation();
	
protected:

	UPROPERTY()
		UCameraComponent* FirstPersonCamera;

};
