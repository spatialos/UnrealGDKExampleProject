// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "TimerComponent.h"
#include "MatchTimerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UMatchTimerComponent : public UTimerComponent
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
};
