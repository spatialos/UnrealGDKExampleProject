// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameStateBase.h"
#include "C10KGameState.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogC10KGameState, Log, All);

UCLASS()
class AC10KGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

	void OutputAIInfos();

private:
	FTimerHandle					AIInfoTimerHandler;
};
