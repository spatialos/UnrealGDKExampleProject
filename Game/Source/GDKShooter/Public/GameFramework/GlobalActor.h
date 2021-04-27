// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GlobalActor.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGlobalActor, Log, All);

USTRUCT()
struct FFpsInfo
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY()
	int64			BeginTime = 0;

	UPROPERTY()
	int64			EndTime = 0;

	UPROPERTY()
	float			FpsAccumulation = 0.0f;

	UPROPERTY()
	int64			FpsTickCount = 0;

	UPROPERTY()
	float			MinFps = 0.0f;

	UPROPERTY()
	float			MaxFps = 0.0f;

	UPROPERTY()
	float			AvgFps = 0.0f;

	void Reset()
	{
		BeginTime = 0;
		EndTime = 0;
		FpsAccumulation = 0.0f;
		FpsTickCount = 0;
		MinFps = 0.0f;
		MaxFps = 0.0f;
		AvgFps = 0.0f;
	}
};

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

	void StatisticNpcInfosTimerHandler();
	void FlushNpcCountToAuthoritativeServer();

	UFUNCTION(CrossServer, Reliable)
	void CrossServerUpdateNpcCount(const FString& RemoteSpatialWorkerId, int32 NpcCount);

	void UpdateFpsInfo(float Fps);
	void FlushFpsInfoTimerHandler();

	UFUNCTION(CrossServer, Reliable)
	void CrossServerUpdateFpsInfo(const FString& RemoteSpatialWorkerId, int32 FpsPeriod, const FFpsInfo& FpsInfo);

	void OutputNpcInfosOnAuthoritativeServer();
	void OutputFpsInfosOnAuthoritativeServer();
	void OutputTimerHandler();

private:

	// yunjie: data used on each server worker
	FTimerHandle						AIInfoTimerHandler;
	FTimerHandle						FpsInfoFlushTimer;

	int32								CachedNpcCount = 0;
	int32								LastFlushNpcCount = 0;

	TMap<int32, FFpsInfo>				FpsInfoMap;


	// yunjie: data only used on authoritative server worker
	FTimerHandle						OutputTimer;

	TMap<FString, int32>							AggregativeServerNpcCountMap;
	TMap<FString, TMap<int32, FFpsInfo>>			AggregativeFpsInfoMap;
};

