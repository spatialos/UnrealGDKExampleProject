// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GDKLogging.h"
#include "Game/GDKGameState.h"
#include "GDKSessionProgress.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "GDKSessionGameState.generated.h"

DECLARE_EVENT_TwoParams(AGDKGameState, FSessionTimerEvent, EGDKSessionProgress, int);

UCLASS(SpatialType = Singleton)
class GDKSHOOTER_API AGDKSessionGameState : public AGDKGameState
{
	GENERATED_BODY()

public:

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	FSessionTimerEvent& OnTimerUpdated() { return TimerEvent; }
	
	UPROPERTY(ReplicatedUsing = OnRep_SessionProgress)
		EGDKSessionProgress SessionProgress;

	UPROPERTY(ReplicatedUsing = OnRep_SessionTimer)
		int SessionTimer;

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UPROPERTY(EditAnywhere, Category = "Timers")
		int LobbySessionLength = 60;

	UPROPERTY(EditAnywhere, Category = "Timers")
		int GameSessionLength = 300;

	UPROPERTY(EditAnywhere, Category = "Timers")
		int ResultsSessionLength = 60;
	
	Worker_EntityId SessionEntityId = 39;
	Worker_ComponentId SessionComponentId = 1000;

	FTimerHandle TickTimer;

	UFUNCTION()
		void OnRep_SessionProgress();

	UFUNCTION()
		void OnRep_SessionTimer();
	
	// Called once per second to advance game timer
	void TickGameTimer();

	// Send a component update to the session manager entity to be picked up by the deployment manager
	void SendStateUpdate(int NewState);

	// Begin progressing through the different stages of game session, if not already started
	void BeginTimer();

	FSessionTimerEvent TimerEvent;
};
