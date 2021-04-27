 // Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GameFramework/GlobalActor.h"
#include "Characters/GDKCharacter.h"
#include "AIController.h"

DEFINE_LOG_CATEGORY(LogGlobalActor);

AGlobalActor::AGlobalActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bAlwaysRelevant = true;
	SetReplicates(true);
}

void AGlobalActor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AGlobalActor::OnAuthorityGained()
{
	Super::OnAuthorityGained();

	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	UE_LOG(LogGlobalActor, Display, TEXT("%s, %s"),
		*SpatialWorkerId, *FString(__FUNCTION__));
}

void AGlobalActor::OnAuthorityLost()
{
	Super::OnAuthorityLost();

	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	UE_LOG(LogGlobalActor, Display, TEXT("%s, %s"),
		*SpatialWorkerId, *FString(__FUNCTION__));
}

void AGlobalActor::Tick(float DeltaTime)
{
}

void AGlobalActor::BeginPlay()
{
	Super::BeginPlay();

	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

	GetWorldTimerManager().SetTimer(AIInfoTimerHandler, this, &AGlobalActor::StatisticNpcInfosTimerHandler, 10.0f, true, 2.0f);
	GetWorldTimerManager().SetTimer(FpsInfoFlushTimer, this, &AGlobalActor::FlushFpsInfoTimerHandler, 5.0f, true, 1.0f);

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(OutputTimer, this, &AGlobalActor::OutputTimerHandler, 10.0f, true, 1.0f);
	}

	FpsInfoMap.FindOrAdd(10);
	FpsInfoMap.FindOrAdd(60);
	FpsInfoMap.FindOrAdd(300);		// yunjie: 5 mins
	FpsInfoMap.FindOrAdd(600);		// yunjie: 10 mins

	UE_LOG(LogGlobalActor, Display, TEXT("%s, %s"),
		*SpatialWorkerId, *FString(__FUNCTION__));
}

void AGlobalActor::StatisticNpcInfosTimerHandler()
{
	int CharacterCount = 0;
	int DroppedCharacterCount = 0;
	for (TObjectIterator<AGDKCharacter> Itr; Itr; ++Itr)
	{
		if (!Itr->HasAuthority())
		{
			continue;
		}

		CharacterCount++;
		if (Itr->GetActorLocation().Z < 0)
		{
			DroppedCharacterCount++;
		}
	}

	int AAIControllerCount = 0;
	for (TObjectIterator<AAIController> Itr; Itr; ++Itr)
	{
		if (!Itr->HasAuthority())
		{
			continue;
		}

		AAIControllerCount++;
	}

	CachedNpcCount = CharacterCount;

	FString SpatialWorkerId = this->GetGameInstance()->IsDedicatedServerInstance() ? TEXT("Server") : TEXT("Client");

	UE_LOG(LogGlobalActor, Display, TEXT("%s, %s, TotalCharacterCount:[%d], DroppedCharacterCount:[%d], TotalAIControllerCount:[%d]"),
		*SpatialWorkerId, *FString(__FUNCTION__), CharacterCount, DroppedCharacterCount, AAIControllerCount);

	FlushNpcCountToAuthoritativeServer();
}

void AGlobalActor::FlushNpcCountToAuthoritativeServer()
{
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

	if (CachedNpcCount != LastFlushNpcCount)
	{
		CrossServerUpdateNpcCount(SpatialWorkerId, CachedNpcCount);
		LastFlushNpcCount = CachedNpcCount;

		UE_LOG(LogGlobalActor, Display, TEXT("%s, %s, NpcCount:[%d]"),
			*SpatialWorkerId, *FString(__FUNCTION__), CachedNpcCount);
	}
}

void AGlobalActor::FlushFpsInfoTimerHandler()
{
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

	for (auto It : FpsInfoMap)
	{
		int32 Period = It.Key;
		FFpsInfo& FpsInfo = It.Value;

		FString BeginTime = FDateTime::FromUnixTimestamp(FpsInfo.BeginTime).ToString();
		FString EndTime = FDateTime::FromUnixTimestamp(FpsInfo.EndTime).ToString();

		UE_LOG(LogGlobalActor, Display, TEXT("%s, %s, Period:[%d], Begin:[%s], End:[%s], MinFps:[%f], AvgFps:[%f], MaxFps:[%f]"),
			*SpatialWorkerId, *FString(__FUNCTION__),
			Period, *BeginTime, *EndTime,
			FpsInfo.MinFps, FpsInfo.AvgFps, FpsInfo.MaxFps);

		CrossServerUpdateFpsInfo(SpatialWorkerId, Period, FpsInfo);
	}
}

void AGlobalActor::CrossServerUpdateNpcCount_Implementation(const FString& RemoteSpatialWorkerId, int32 NpcCount)
{
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	AggregativeServerNpcCountMap.FindOrAdd(RemoteSpatialWorkerId) = NpcCount;
}

void AGlobalActor::UpdateFpsInfo(float Fps)
{
	FDateTime DateTime = FDateTime::Now();
	int64 NowTs = DateTime.ToUnixTimestamp();

	for (TPair<int32, FFpsInfo>& It : FpsInfoMap)
	{
		int32 Period = It.Key;
		FFpsInfo& FpsInfo = It.Value;

		if (FpsInfo.EndTime - FpsInfo.BeginTime >= Period)
		{
			FpsInfo.Reset();
		}

		if (FpsInfo.BeginTime == 0)
		{
			FpsInfo.BeginTime = NowTs;
		}

		FpsInfo.EndTime = NowTs;

		FpsInfo.FpsAccumulation += Fps;
		FpsInfo.FpsTickCount++;

		if (Fps < FpsInfo.MinFps || 0 == FpsInfo.MinFps)
		{
			FpsInfo.MinFps = Fps;
		}

		if (Fps > FpsInfo.MaxFps)
		{
			FpsInfo.MaxFps = Fps;
		}

		FpsInfo.AvgFps = FpsInfo.FpsAccumulation /  FpsInfo.FpsTickCount;
	}
}

void AGlobalActor::CrossServerUpdateFpsInfo_Implementation(const FString& RemoteSpatialWorkerId, int32 FpsPeriod, const FFpsInfo& FpsInfo)
{
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	TMap<int32, FFpsInfo>& FoundFpsInfoMap = AggregativeFpsInfoMap.FindOrAdd(RemoteSpatialWorkerId);
	FoundFpsInfoMap.FindOrAdd(FpsPeriod) = FpsInfo;
}

void AGlobalActor::OutputNpcInfosOnAuthoritativeServer()
{
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	int32 TotalNpcCount = 0;

	for (auto It : AggregativeServerNpcCountMap)
	{
		UE_LOG(LogGlobalActor, Warning, TEXT("%s, %s, SpatialWorkerId:[%s], NpcCount:[%d]"),
			*SpatialWorkerId, *FString(__FUNCTION__), *It.Key, It.Value);
		TotalNpcCount += It.Value;
	}

	UE_LOG(LogGlobalActor, Warning, TEXT("%s, %s, TotalNpcCount:[%d]"),
		*SpatialWorkerId, *FString(__FUNCTION__), TotalNpcCount);
}

void AGlobalActor::OutputFpsInfosOnAuthoritativeServer()
{
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	for (auto It1 : AggregativeFpsInfoMap)
	{
		for (auto It2 : It1.Value)
		{
			int32 Period = It2.Key;
			FFpsInfo FpsInfo = It2.Value;
			FString BeginTime = FDateTime::FromUnixTimestamp(FpsInfo.BeginTime).ToString();
			FString EndTime = FDateTime::FromUnixTimestamp(FpsInfo.EndTime).ToString();

			UE_LOG(LogGlobalActor, Warning, TEXT("%s, %s, SpatialWorkerId:[%s], Period:[%d], Begin:[%s], End:[%s], MinFps:[%f], AvgFps:[%f], MaxFps:[%f]"),
				*SpatialWorkerId, *FString(__FUNCTION__), *It1.Key,
				Period, *BeginTime, *EndTime,
				FpsInfo.MinFps, FpsInfo.AvgFps, FpsInfo.MaxFps);
		}
	}
}

void AGlobalActor::OutputTimerHandler()
{
	OutputNpcInfosOnAuthoritativeServer();
	OutputFpsInfosOnAuthoritativeServer();
}

