// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MatchStateComponent.generated.h"

UENUM(BlueprintType)
enum class EMatchState : uint8
{
	PreGame			UMETA(DisplayName = "PreGame"),
	InGame			UMETA(DisplayName = "InGame"),
	PostGame		UMETA(DisplayName = "PostGame")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMatchEvent, EMatchState, CurrentState);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UMatchStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMatchStateComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable)
		FMatchEvent MatchEvent;
	
	UFUNCTION(BlueprintCallable)
		void SetMatchState(EMatchState NewState);

	UFUNCTION(BlueprintPure)
		EMatchState GetCurrentState() { return CurrentState; }

protected:

	UPROPERTY(ReplicatedUsing=OnRep_State)
		EMatchState CurrentState;

	UFUNCTION()
		void OnRep_State() { MatchEvent.Broadcast(CurrentState); }
};
