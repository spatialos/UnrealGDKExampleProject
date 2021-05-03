// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Game/Components/SkillComponent.h"
#include "Characters/GDKCharacter.h"
#include "Engine/World.h"
#include "GDKLogging.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogSkillComponent);

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	FSkillDesc FireBallSkillDesc;
	FireBallSkillDesc.id = SkillId_FireBall;
	FireBallSkillDesc.SkillType = SkillType_SingleTarget;
	FireBallSkillDesc.SkillEffects.Add(SkillEffect_FireBall);
	FireBallSkillDesc.SkillBuffs.Add(SKillBuff_Firing);
	SkillTable.Add(SkillId_FireBall, FireBallSkillDesc);

	FSkillDesc StormSkillDesc;
	StormSkillDesc.id = SkillId_Storm;
	StormSkillDesc.SkillType = SkillType_CircleArea;
	StormSkillDesc.SkillEffects.Add(SkillEffect_Storm);
	StormSkillDesc.SkillBuffs.Add(SKillBuff_Frozen);
	SkillTable.Add(SkillId_Storm, StormSkillDesc);

	FSkillDesc PosionSkillDesc;
	PosionSkillDesc.id = SkillId_Poison;
	PosionSkillDesc.SkillType = SkillType_MultipleTargets;
	PosionSkillDesc.SkillEffects.Add(SkillEffect_Poison);
	PosionSkillDesc.SkillBuffs.Add(SkillBuff_Poisonous);
	SkillTable.Add(SkillId_Poison, PosionSkillDesc);
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USkillComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void USkillComponent::UseSkill(int32 SkillId, TArray<AGDKCharacter*> Targets)
{
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

	const FSkillDesc* SkillDesc = SkillTable.Find(SkillId);
	if (nullptr == SkillDesc)
	{
		UE_LOG(LogSkillComponent, Error, TEXT("%s, %s, Invalid SkillId:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillId, Targets.Num());
		return;
	}

	UE_LOG(LogSkillComponent, Display, TEXT("%s, %s, Using SkillId:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillId, Targets.Num());

	for (auto Effect : SkillDesc->SkillEffects)
	{
		ProcessSkillEffect(Effect, Targets);
	}

	for (auto Buff : SkillDesc->SkillBuffs)
	{
		ProcessSkillBuff(Buff, Targets);
	}
}

void USkillComponent::ProcessSkillEffect(int32 SkillEffect, TArray<AGDKCharacter*> Targets)
{
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

	UE_LOG(LogSkillComponent, Display, TEXT("%s, %s, Using SkillEffect:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillEffect, Targets.Num());

	switch (SkillEffect)
	{
	case SkillEffect_FireBall:
	{

	}
	break;

	case SkillEffect_Storm:
	{

	}
	break;

	case SkillEffect_Poison:
	{

	}
	break;

	default:
	{
		UE_LOG(LogSkillComponent, Error, TEXT("%s, %s, Invalid SkillEffect:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillEffect, Targets.Num());
	}

	}

}

void USkillComponent::ProcessSkillBuff(int32 SkillBuff, TArray<AGDKCharacter*> Targets)
{
	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

	UE_LOG(LogSkillComponent, Display, TEXT("%s, %s, Using SkillBuff:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillBuff, Targets.Num());

	switch (SkillBuff)
	{
	case SKillBuff_Firing:
	{

	}
	break;

	case SKillBuff_Frozen:
	{

	}
	break;

	case SkillBuff_Poisonous:
	{

	}
	break;

	default:
	{
		UE_LOG(LogSkillComponent, Error, TEXT("%s, %s, Invalid SkillBuff:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillBuff, Targets.Num());
	}

	}
}

