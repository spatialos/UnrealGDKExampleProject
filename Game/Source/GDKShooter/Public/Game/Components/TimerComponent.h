// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "TimerComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTimerEvent, int, CurrentTimer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTimerFinishedEvent);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UTimerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTimerComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
		void StartTimer();
	UFUNCTION(BlueprintCallable)
		void ResumeTimer();
	UFUNCTION(BlueprintCallable)
		void SetTimer(int32 NewValue);
	UFUNCTION(BlueprintCallable)
		void StopTimer();

	UPROPERTY(BlueprintAssignable)
		FTimerEvent OnTimer;
	UPROPERTY(BlueprintAssignable)
		FTimerFinishedEvent OnTimerFinished;

	UFUNCTION(BlueprintPure)
		int32 GetTimer() { return TimeLeft; }

protected:
	void BeginPlay();

	UPROPERTY(EditDefaultsOnly)
		bool bAutoStart = false;
	UPROPERTY(EditDefaultsOnly)
		int32 DefaultTimerDuration = 300;

	UPROPERTY(Replicated, BlueprintReadOnly)
		bool bIsTimerRunning = false;
	UPROPERTY(ReplicatedUsing = OnRep_Timer, BlueprintReadOnly)
		int32 TimeLeft;
	UPROPERTY(ReplicatedUsing = OnRep_TimerFinished, BlueprintReadOnly)
		bool bHasTimerFinished = false;

	UFUNCTION()
		void DecrementTimer();
	UFUNCTION()
		void OnRep_Timer();
	UFUNCTION()
		void OnRep_TimerFinished();

	FTimerHandle TimerHandle;
};
