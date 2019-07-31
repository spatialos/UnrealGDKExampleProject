// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "TimerComponent.h"
#include "LobbyTimerComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API ULobbyTimerComponent : public UTimerComponent
{
	GENERATED_BODY()

public:	

	UFUNCTION(BlueprintCallable)
		void InformOfPlayerCount(int32 PlayerCount);
	UPROPERTY(EditDefaultsOnly)
		int32 MinimumPlayersToStartCountdown = 1;

protected:
	virtual void BeginPlay() override;

	
};
