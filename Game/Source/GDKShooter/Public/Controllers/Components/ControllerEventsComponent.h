// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "ControllerEventsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActorEvent, const AActor*, Actor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UControllerEventsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UControllerEventsComponent();

	void KilledBy(const AActor* Killer);
		
	UPROPERTY(BlueprintAssignable)
		FActorEvent KillEvent;
};
