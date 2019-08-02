// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/PlayerState.h"
#include "ScorePublisher.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FKillEvent, int32, KillerId, int32, VictimId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeathOfState, APlayerState*, VictimState);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UScorePublisher : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable)
	void PublishKill(int32 KillerId, APlayerState* VictimState) {
		KillEvent.Broadcast(KillerId, VictimState->PlayerId);
	}

	UFUNCTION(BlueprintCallable)
		void PublishDeath(APlayerState* VictimState) {
		DeathEvent.Broadcast(VictimState);
	}
	
	UPROPERTY(BlueprintAssignable)
		FKillEvent KillEvent;
	UPROPERTY(BlueprintAssignable)
		FDeathOfState DeathEvent;

};
