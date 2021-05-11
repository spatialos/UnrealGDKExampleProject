// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Game/Components/SkillEffectComponent.h"
#include "Characters/GDKCharacter.h"
#include "Engine/World.h"
#include "GDKLogging.h"
#include "Net/UnrealNetwork.h"
#include "Characters/Components/GDKMovementComponent.h"

DEFINE_LOG_CATEGORY(LogSkillComponent);

USkillEffectComponent::USkillEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	// SetIsReplicated(true);

	FSkillDesc FireBallSkillDesc;
	FireBallSkillDesc.id = SkillId_FireBall;
	FireBallSkillDesc.SkillType = SkillType_SingleTarget;
	FireBallSkillDesc.SkillEffects.Add(SkillEffect_Instant_FireBall);
	FireBallSkillDesc.SkillEffects.Add(SKillEffect_Buff_Firing);
	FireBallSkillDesc.Probability = 0;
	SkillTable.Add(SkillId_FireBall, FireBallSkillDesc);

	FSkillDesc StormSkillDesc;
	StormSkillDesc.id = SkillId_Storm;
	StormSkillDesc.SkillType = SkillType_CircleArea;
	StormSkillDesc.SkillEffects.Add(SkillEffect_Instant_Storm);
	StormSkillDesc.SkillEffects.Add(SKillEffect_Buff_Frozen);
	StormSkillDesc.Probability = 100;
	SkillTable.Add(SkillId_Storm, StormSkillDesc);

	FSkillDesc PosionSkillDesc;
	PosionSkillDesc.id = SkillId_Poison;
	PosionSkillDesc.SkillType = SkillType_MultipleTargets;
	PosionSkillDesc.SkillEffects.Add(SkillEffect_Instant_Poison);
	PosionSkillDesc.SkillEffects.Add(SkillEffect_Buff_Poisonous);
	PosionSkillDesc.Probability = 0;
	SkillTable.Add(SkillId_Poison, PosionSkillDesc);
}

void USkillEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());

	OwnerCharacter->GetWorldTimerManager().SetTimer(EffectTimer, this, &USkillEffectComponent::ProcessEffectTimer, 1.0f, true, 1.0f);
}

void USkillEffectComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USkillEffectComponent, aaa);
	DOREPLIFETIME(USkillEffectComponent, EffectStatus);
}

void USkillEffectComponent::UseSkillRandomly()
{
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();

	AC10KGameState* GameState = Cast<AC10KGameState>(GetWorldWrapper()->GetGameState());
	UClass* NpcClass = GameState->NpcClass;
	UClass* NpcSpawnerClass = GameState->NpcSpawnerClass;
	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());

	TArray<AGDKCharacter*> CharactersInRange;

	TArray<AActor *> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorldWrapper(), AGDKCharacter::StaticClass(), OutActors);

	for (TObjectIterator<AGDKCharacter> Itr; Itr; ++Itr)
	{
		AGDKCharacter* Character = *Itr;
		if (!Character->HasAuthority())
		{
			continue;
		}

		if (Character == OwnerCharacter)
		{
			continue;
		}

		if (FVector::Dist2D(OwnerCharacter->GetActorLocation(), Character->GetActorLocation()) <= TargetsRadius)
		{
			CharactersInRange.Add(Character);
		}
	}

	const int32 OriginalRandValue = FMath::RandRange(1, 100);
	int32 RandValue = OriginalRandValue;

	FSkillDesc RandedSkill;
	for (auto Skill : SkillTable)
	{
		if (RandValue <= Skill.Value.Probability)
		{
			RandedSkill = Skill.Value;
			break;
		}

		RandValue -= Skill.Value.Probability;
	}

	if (false == RandedSkill.IsValid())
	{
		UE_LOG(LogSkillComponent, Error, TEXT("%s, %s, Invalid randed skill, RandValue:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), OriginalRandValue);
		return;
	}

	UE_LOG(LogSkillComponent, Display, TEXT("%s, %s, RandValue:[%d], RandedSkill:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), OriginalRandValue, RandedSkill.id);

	UseSkillOnServer(RandedSkill.id, CharactersInRange);
}

void USkillEffectComponent::UseSkillOnServer(int32 SkillId, TArray<AGDKCharacter*> Targets)
{
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();

	const FSkillDesc* SkillDesc = SkillTable.Find(SkillId);
	if (nullptr == SkillDesc)
	{
		UE_LOG(LogSkillComponent, Error, TEXT("%s, %s, Invalid SkillId:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillId, Targets.Num());
		return;
	}

	UE_LOG(LogSkillComponent, Display, TEXT("%s, %s, Using SkillId:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillId, Targets.Num());

	for (auto Effect : SkillDesc->SkillEffects)
	{
		ProcessSkillEffectOnServer(Effect, Targets);
	}

	ClientUseSkill(SkillId, Targets);
}

void USkillEffectComponent::ProcessSkillEffectOnServer(int32 SkillEffect, TArray<AGDKCharacter*> Targets)
{
	AC10KGameState* GameState = Cast<AC10KGameState>(GetWorldWrapper()->GetGameState());
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();
	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());
	int64 NowTs = GameState->NowTs;

	UE_LOG(LogSkillComponent, Display, TEXT("%s, %s, Using SkillEffect:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillEffect, Targets.Num());

	for (auto Character : Targets)
	{
		if (Character->HasAuthority())
		{
			UGDKMovementComponent* TargetMovementComponent = Cast<UGDKMovementComponent>(Character->GetCharacterMovement());

			Character->CurrentNpcSpawnerIdx++;
			Character->GetSkillComponent()->aaa++;
			Character->GetSkillComponent()->EffectStatus[SkillEffect].EffectId = SkillEffect;
			Character->GetSkillComponent()->EffectStatus[SkillEffect].ExpireTime = NowTs + SKILL_EFFECT_BUFF_TIME;
			Character->GetSkillComponent()->UpdateEffectStatus();

			// FDamageEvent De;
			// Character->TakeDamageCrossServer(500, De, Character->GetController(), OwnerCharacter);
		}
	}
}

void USkillEffectComponent::ClientUseSkill_Implementation(int32 SkillId, const TArray<AGDKCharacter*>& Targets)
{
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();

	const FSkillDesc* SkillDesc = SkillTable.Find(SkillId);
	if (nullptr == SkillDesc)
	{
		UE_LOG(LogSkillComponent, Error, TEXT("%s, %s, Invalid SkillId:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillId, Targets.Num());
		return;
	}

	UE_LOG(LogSkillComponent, Display, TEXT("%s, %s, Using SkillId:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillId, Targets.Num());

	for (auto Effect : SkillDesc->SkillEffects)
	{
		ProcessSkillEffectOnClient(Effect, Targets);
	}
}

void USkillEffectComponent::ProcessSkillEffectOnClient(int32 SkillEffect, const TArray<AGDKCharacter*>& Targets)
{
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();

	UE_LOG(LogSkillComponent, Display, TEXT("%s, %s, Using SkillEffect:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillEffect, Targets.Num());

	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());
}

void USkillEffectComponent::ProcessEffectTimer()
{
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();
	UE_LOG(LogSkillComponent, Verbose, TEXT("%s, %s"), *SpatialWorkerId, *FString(__FUNCTION__));

	UpdateEffectStatus();
}

void USkillEffectComponent::OnRep_EffectStatus()
{
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();
	UE_LOG(LogSkillComponent, Display, TEXT("%s, %s"), *SpatialWorkerId, *FString(__FUNCTION__));
	UpdateEffectStatus();
}

void USkillEffectComponent::UpdateEffectStatus()
{
	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();
	AC10KGameState* GameState = Cast<AC10KGameState>(GetWorldWrapper()->GetGameState());
	UGDKMovementComponent* MovementComponent = Cast<UGDKMovementComponent>(OwnerCharacter->GetCharacterMovement());
	int64 NowTs = GameState->NowTs;

	UE_LOG(LogSkillComponent, Verbose, TEXT("%s, %s"), *SpatialWorkerId, *FString(__FUNCTION__));

	for (int32 Idx = 0; Idx < SkillEffect_Max; ++Idx)
	{
		if (EffectStatus[Idx].ShouldBeTriggered(NowTs))
		{
			TriggerEffect(EffectStatus[Idx].EffectId);
			EffectStatus[Idx].Trigger();
		}
		else if (EffectStatus[Idx].IsValid(NowTs))
		{
			UpdateEffect(EffectStatus[Idx].EffectId);
		}
		else if (EffectStatus[Idx].ShouldBeCleared(NowTs))
		{
			ClearEffect(EffectStatus[Idx].EffectId);
			EffectStatus[Idx].Reset();
		}
	}
}

void USkillEffectComponent::TriggerEffect(int32 SkillBuff)
{
	AC10KGameState* GameState = Cast<AC10KGameState>(GetWorldWrapper()->GetGameState());
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();
	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());
	UGDKMovementComponent* MovementComponent = Cast<UGDKMovementComponent>(OwnerCharacter->GetCharacterMovement());
	int64 NowTs = GameState->NowTs;

	UE_LOG(LogSkillComponent, Display, TEXT("%s, %s, Using SkillBuff:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillBuff);

	switch (SkillBuff)
	{
	case SkillEffect_Instant_FireBall:
	{

	}
	break;

	case SkillEffect_Instant_Storm:
	{

	}
	break;

	case SkillEffect_Instant_Poison:
	{

	}
	break;

	case SKillEffect_Buff_Firing:
	{

	}
	break;

	case SKillEffect_Buff_Frozen:
	{
		float OldWalkSpeed = MovementComponent->MaxJogSpeed;
		MovementComponent->MaxJogSpeed *= SKILL_EFFECT_BUFF_FRONZEN_SPEED_RATIO;

		UE_LOG(LogSkillComponent, Display, TEXT("%s, %s, Using SkillBuff:[%d], OldWalkSpeed:[%f], NewWalkSpeed:[%f]"),
			*SpatialWorkerId, *FString(__FUNCTION__), SkillBuff, OldWalkSpeed, MovementComponent->MaxJogSpeed);
	}
	break;

	case SkillEffect_Buff_Poisonous:
	{

	}
	break;

	default:
	{
	}
	}
}

void USkillEffectComponent::UpdateEffect(int32 SkillBuff)
{
	AC10KGameState* GameState = Cast<AC10KGameState>(GetWorldWrapper()->GetGameState());
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();
	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());
	UGDKMovementComponent* MovementComponent = Cast<UGDKMovementComponent>(OwnerCharacter->GetCharacterMovement());
	int64 NowTs = GameState->NowTs;

	UE_LOG(LogSkillComponent, Display, TEXT("%s, %s, Using SkillBuff:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillBuff);

	switch (SkillBuff)
	{
	case SkillEffect_Instant_FireBall:
	{

	}
	break;

	case SkillEffect_Instant_Storm:
	{

	}
	break;

	case SkillEffect_Instant_Poison:
	{

	}
	break;

	case SKillEffect_Buff_Firing:
	{

	}
	break;

	case SKillEffect_Buff_Frozen:
	{
	}
	break;

	case SkillEffect_Buff_Poisonous:
	{

	}
	break;

	default:
	{
	}
	}
}

void USkillEffectComponent::ClearEffect(int32 SkillBuff)
{
	AC10KGameState* GameState = Cast<AC10KGameState>(GetWorldWrapper()->GetGameState());
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();
	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());
	UGDKMovementComponent* MovementComponent = Cast<UGDKMovementComponent>(OwnerCharacter->GetCharacterMovement());
	int64 NowTs = GameState->NowTs;

	UE_LOG(LogSkillComponent, Display, TEXT("%s, %s, Using SkillBuff:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillBuff);

	switch (SkillBuff)
	{
	case SkillEffect_Instant_FireBall:
	{

	}
	break;

	case SkillEffect_Instant_Storm:
	{

	}
	break;

	case SkillEffect_Instant_Poison:
	{

	}
	break;

	case SKillEffect_Buff_Firing:
	{

	}
	break;

	case SKillEffect_Buff_Frozen:
	{
		float OldWalkSpeed = MovementComponent->MaxJogSpeed;
		MovementComponent->MaxJogSpeed /= SKILL_EFFECT_BUFF_FRONZEN_SPEED_RATIO;

		UE_LOG(LogSkillComponent, Display, TEXT("%s, %s, Using SkillBuff:[%d], OldWalkSpeed:[%f], NewWalkSpeed:[%f]"),
			*SpatialWorkerId, *FString(__FUNCTION__), SkillBuff, OldWalkSpeed, MovementComponent->MaxJogSpeed);
	}
	break;

	case SkillEffect_Buff_Poisonous:
	{

	}
	break;

	default:
	{
	}
	}
}

void USkillEffectComponent::OnRep_aaa()
{
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();
	UE_LOG(LogSkillComponent, Display, TEXT("%s, %s"), *SpatialWorkerId, *FString(__FUNCTION__));
}

