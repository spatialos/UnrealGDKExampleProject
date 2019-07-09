// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScorePublisher.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FKillEvent, int32, KillerId, int32, VictimId);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UScorePublisher : public UActorComponent
{
	GENERATED_BODY()

public:	
	void PublishKill(int32 KillerId, int32 VictimId) { KillEvent.Broadcast(KillerId, VictimId); }
	
	UPROPERTY(BlueprintAssignable)
		FKillEvent KillEvent;

};
