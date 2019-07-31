// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpawnRequestPublisher.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnRequest, APlayerController*, Controller);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API USpawnRequestPublisher : public UActorComponent
{
	GENERATED_BODY()

public:
	USpawnRequestPublisher();

	UPROPERTY(BlueprintAssignable)
		FSpawnRequest OnSpawnRequest;

	void RequestSpawn(APlayerController* Controller) { OnSpawnRequest.Broadcast(Controller); }
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		bool bAutoConnect;

};
