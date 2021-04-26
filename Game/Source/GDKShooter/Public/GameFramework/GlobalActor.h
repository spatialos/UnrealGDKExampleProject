// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GlobalActor.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGlobalActor, Log, All);

UCLASS(SpatialType = ServerOnly)
class AGlobalActor : public AActor
{
	GENERATED_BODY()

public:
	AGlobalActor();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void OnAuthorityGained() override;
	virtual void OnAuthorityLost() override;

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	void FlushNpcCountToAuthoritativeServer();

	UFUNCTION(CrossServer, Reliable)
	void CrossServerUpdateNpcCount(const FString& RemoteSpatialWorkerId, int32 NpcCount);

public:
	int32						CachedNpcCount = 0;
	int32						LastFlushNpcCount = 0;
	TMap<FString, int32>		ServerNpcCountMap;

private:
	FTimerHandle					NpcCountFlushTimer;
};

