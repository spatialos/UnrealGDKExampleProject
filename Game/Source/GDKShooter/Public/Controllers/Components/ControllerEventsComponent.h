// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "ControllerEventsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FControllerEvent, const AController*, Controller);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FKillDetailsEvent, const FString&, VictimName, int32, VictimId);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UControllerEventsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UControllerEventsComponent();

	UFUNCTION(CrossServer, Reliable)
	void Death(const AController* Killer);

	UFUNCTION(CrossServer, Reliable)
	void Kill(const AController* Victim);

	UPROPERTY(BlueprintAssignable)
	FControllerEvent DeathEvent;

	UPROPERTY(BlueprintAssignable)
	FControllerEvent KillEvent;

	UFUNCTION(Client, Reliable)
	void ClientInformOfKill(const FString& VictimName, int32 VictimId);

	UFUNCTION(Client, Reliable)
	void ClientInformOfDeath(const FString& KillerName, int32 KillerId);
	
	UPROPERTY(BlueprintAssignable)
	FKillDetailsEvent KillDetailsEvent;

	UPROPERTY(BlueprintAssignable)
	FKillDetailsEvent DeathDetailsEvent;
};
