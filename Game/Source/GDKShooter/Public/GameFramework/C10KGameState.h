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
	AC10KGameState();

	virtual void OnAuthorityGained() override;
	virtual void OnAuthorityLost() override;

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	void OutputAIInfos();

	UClass*				NpcSpawnerClass;
	UClass*				NpcClass;

private:
	FTimerHandle					AIInfoTimerHandler;

	float							FrameTime;
	int32							FrameCount;
};
