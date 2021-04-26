 // Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GameFramework/GlobalActor.h"

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
	UE_LOG(LogGlobalActor, Display, TEXT("%s, %s, Name:[%s]"),
		*SpatialWorkerId, *FString(__FUNCTION__), *this->GetName());
}

void AGlobalActor::OnAuthorityLost()
{
	Super::OnAuthorityLost();

	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	UE_LOG(LogGlobalActor, Display, TEXT("%s, %s, Name:[%s]"),
		*SpatialWorkerId, *FString(__FUNCTION__), *this->GetName());
}

void AGlobalActor::Tick(float DeltaTime)
{
}

void AGlobalActor::BeginPlay()
{
	Super::BeginPlay();

	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

	GetWorldTimerManager().SetTimer(NpcCountFlushTimer, this, &AGlobalActor::FlushNpcCountToAuthoritativeServer, 10.0f, true, 1.0f);

	UE_LOG(LogGlobalActor, Display, TEXT("%s, %s, Name:[%s]"),
		*SpatialWorkerId, *FString(__FUNCTION__), *this->GetName());
}

void AGlobalActor::FlushNpcCountToAuthoritativeServer()
{
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

	if (CachedNpcCount != LastFlushNpcCount)
	{
		CrossServerUpdateNpcCount(SpatialWorkerId, CachedNpcCount);
		LastFlushNpcCount = CachedNpcCount;

		UE_LOG(LogGlobalActor, Display, TEXT("%s, %s, Name:[%s], NpcCount:[%d]"),
			*SpatialWorkerId, *FString(__FUNCTION__), *this->GetName(), CachedNpcCount);
	}
}

void AGlobalActor::CrossServerUpdateNpcCount_Implementation(const FString& RemoteSpatialWorkerId, int32 NpcCount)
{
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

	ServerNpcCountMap.FindOrAdd(RemoteSpatialWorkerId) = NpcCount;

	int32 TotalNpcCount = 0;

	for (auto It : ServerNpcCountMap)
	{
		UE_LOG(LogGlobalActor, Display, TEXT("%s, %s, Name:[%s], SpatialWorkerId:[%s], NpcCount:[%d]"),
			*SpatialWorkerId, *FString(__FUNCTION__), *this->GetName(), *It.Key, It.Value);
		TotalNpcCount += It.Value;
	}

	UE_LOG(LogGlobalActor, Display, TEXT("%s, %s, Name:[%s], TotalNpcCount:[%d]"),
		*SpatialWorkerId, *FString(__FUNCTION__), *this->GetName(), TotalNpcCount);
}

