// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerPublisher.h"
#include "PlayerCountingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerCountEvent, int32, PlayerCount);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UPlayerCountingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerCountingComponent();

	UFUNCTION(BlueprintPure)
	int32 PlayerCount() { return ConnectedPlayerCount; }

	UPROPERTY(BlueprintAssignable)
	FPlayerCountEvent PlayerCountEvent;

	UFUNCTION(BlueprintCallable)
	void PlayerEvent(APlayerState* PlayerState, EPlayerProgress Progress);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_ConnectedPlayerCount)
	int32 ConnectedPlayerCount;

	UFUNCTION()
	void OnRep_ConnectedPlayerCount();
};
