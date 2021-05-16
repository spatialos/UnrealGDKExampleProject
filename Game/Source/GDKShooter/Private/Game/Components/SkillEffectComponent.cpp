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
	FireBallSkillDesc.SkillEffects.Add(FSkillEffectDesc{ SkillEffect_Instant_FireBall, 2 });
	FireBallSkillDesc.SkillEffects.Add(FSkillEffectDesc{ SKillEffect_Buff_Firing, 5});
	FireBallSkillDesc.Probability = 34;
	SkillTable.Add(SkillId_FireBall, FireBallSkillDesc);

	FSkillDesc StormSkillDesc;
	StormSkillDesc.id = SkillId_Storm;
	StormSkillDesc.SkillType = SkillType_CircleArea;
	StormSkillDesc.SkillEffects.Add(FSkillEffectDesc{ SkillEffect_Instant_Storm, 2 });
	StormSkillDesc.SkillEffects.Add(FSkillEffectDesc{ SKillEffect_Buff_Frozen, 8 });
	StormSkillDesc.Probability = 33;
	SkillTable.Add(SkillId_Storm, StormSkillDesc);

	FSkillDesc PosionSkillDesc;
	PosionSkillDesc.id = SkillId_Poison;
	PosionSkillDesc.SkillType = SkillType_MultipleTargets;
	PosionSkillDesc.SkillEffects.Add(FSkillEffectDesc{ SkillEffect_Instant_Poison, 2 });
	PosionSkillDesc.SkillEffects.Add(FSkillEffectDesc{ SkillEffect_Buff_Poisonous, 15 });
	PosionSkillDesc.Probability = 33;
	SkillTable.Add(SkillId_Poison, PosionSkillDesc);
}

void USkillEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());
	GetEffectPlaneComponent()->SetVisibility(false);

	OwnerCharacter->GetWorldTimerManager().SetTimer(EffectTimer, this, &USkillEffectComponent::ProcessEffectTimer, 1.0f, true, 1.0f);
}

void USkillEffectComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

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

		if (!Character->GetSkillComponent())
		{
			continue;
		}

		if (Character->GetSkillComponent()->HasEffects())
		{
			continue;
		}

		if (FVector::Dist2D(OwnerCharacter->GetActorLocation(), Character->GetActorLocation()) <= TargetsRadius)
		{
			CharactersInRange.Add(Character);
		}
	}

	if (!CharactersInRange.Num())
	{
		return;
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

	UE_LOG(LogSkillComponent, Verbose, TEXT("%s, %s, RandValue:[%d], RandedSkill:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), OriginalRandValue, RandedSkill.id);

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

	UE_LOG(LogSkillComponent, Verbose, TEXT("%s, %s, Using SkillId:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillId, Targets.Num());

	for (auto Effect : SkillDesc->SkillEffects)
	{
		ProcessSkillEffectOnServer(Effect, Targets);
	}

	NetMulticastUseSkill(SkillId, Targets);
}

void USkillEffectComponent::ProcessSkillEffectOnServer(const FSkillEffectDesc& SkillEffect, TArray<AGDKCharacter*> Targets)
{
	AC10KGameState* GameState = Cast<AC10KGameState>(GetWorldWrapper()->GetGameState());
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();
	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());
	int64 NowTs = GameState->NowTs;

	UE_LOG(LogSkillComponent, Verbose, TEXT("%s, %s, Using SkillEffect:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillEffect.SkillEffectId, Targets.Num());

	for (auto Character : Targets)
	{
		if (Character->HasAuthority())
		{
			UGDKMovementComponent* TargetMovementComponent = Cast<UGDKMovementComponent>(Character->GetCharacterMovement());

			Character->GetSkillComponent()->EffectStatus[SkillEffect.SkillEffectId].Causer = OwnerCharacter;
			Character->GetSkillComponent()->EffectStatus[SkillEffect.SkillEffectId].EffectId = SkillEffect.SkillEffectId;
			Character->GetSkillComponent()->EffectStatus[SkillEffect.SkillEffectId].ExpireTime = NowTs + SkillEffect.SkillEffectTime;
			Character->GetSkillComponent()->UpdateEffectStatus();

			// FDamageEvent De;
			// Character->TakeDamageCrossServer(500, De, Character->GetController(), OwnerCharacter);
		}
	}
}

void USkillEffectComponent::NetMulticastUseSkill_Implementation(int32 SkillId, const TArray<AGDKCharacter*>& Targets)
{
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();

	const FSkillDesc* SkillDesc = SkillTable.Find(SkillId);
	if (nullptr == SkillDesc)
	{
		UE_LOG(LogSkillComponent, Error, TEXT("%s, %s, Invalid SkillId:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillId, Targets.Num());
		return;
	}

	UE_LOG(LogSkillComponent, Verbose, TEXT("%s, %s, Using SkillId:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillId, Targets.Num());

	for (auto Effect : SkillDesc->SkillEffects)
	{
		ProcessSkillEffectOnClient(Effect, Targets);
	}
}

void USkillEffectComponent::ProcessSkillEffectOnClient(const FSkillEffectDesc& SkillEffect, const TArray<AGDKCharacter*>& Targets)
{
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();

	UE_LOG(LogSkillComponent, Verbose, TEXT("%s, %s, Using SkillEffect:[%d], TargetCount:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillEffect.SkillEffectId, Targets.Num());

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
	UE_LOG(LogSkillComponent, Verbose, TEXT("%s, %s"), *SpatialWorkerId, *FString(__FUNCTION__));
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
		if (EffectStatus[Idx].IsEmpty())
		{
			continue;
		}

		if (!EffectStatus[Idx].SanityCheck())
		{
			// yunjie: some actor reference might be invalidated, so just clear the effect here
			ClearEffect(EffectStatus[Idx]);
			EffectStatus[Idx].Reset();

			continue;
		}

		if (EffectStatus[Idx].ShouldBeTriggered(NowTs))
		{
			TriggerEffect(EffectStatus[Idx]);
			EffectStatus[Idx].Trigger();
		}
		else if (EffectStatus[Idx].ShouldBeCleared(NowTs))
		{
			ClearEffect(EffectStatus[Idx]);
			EffectStatus[Idx].Reset();
		}
		else
		{
			UpdateEffect(EffectStatus[Idx]);
		}
	}
}

void USkillEffectComponent::TriggerEffect(FSkillEffectStatus& SkillEffect)
{
	AC10KGameState* GameState = Cast<AC10KGameState>(GetWorldWrapper()->GetGameState());
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();
	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());
	UGDKMovementComponent* MovementComponent = Cast<UGDKMovementComponent>(OwnerCharacter->GetCharacterMovement());
	int64 NowTs = GameState->NowTs;

	UCapsuleComponent* CapsuleComponent =
			Cast<UCapsuleComponent>(OwnerCharacter->GetComponentByClass(UCapsuleComponent::StaticClass()));

	UE_LOG(LogSkillComponent, Verbose, TEXT("%s, %s, Using SkillEffect:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillEffect.EffectId);

	switch (SkillEffect.EffectId)
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
		if (!GetWorldWrapper()->GetGameInstance()->IsDedicatedServerInstance())
		{
			FVector SkillLocation = SkillEffect.Causer->GetActorLocation();
			FRotator SkillRotation = SkillEffect.Causer->GetActorRotation();
			AActor* Fireball = GetWorldWrapper()->SpawnActor<AActor>(GameState->FireballClass,
				SkillLocation,
				SkillRotation);

			UProjectileMovementComponent* ProjectileMovementComponent =
				Cast<UProjectileMovementComponent>(Fireball->GetComponentByClass(UProjectileMovementComponent::StaticClass()));

			ProjectileMovementComponent->HomingTargetComponent = CapsuleComponent;
			ProjectileMovementComponent->bIsHomingProjectile = true;
			ProjectileMovementComponent->HomingAccelerationMagnitude = 25000;

			SkillEffect.ClientEffectActors.Add(Fireball);
		}

		GetEffectPlaneComponent()->SetVisibility(true);
		GetEffectPlaneComponent()->SetMaterial(0, FiringMaterialBody);
	}
	break;

	case SKillEffect_Buff_Frozen:
	{
		float OldWalkSpeed = MovementComponent->MaxJogSpeed;
		MovementComponent->MaxJogSpeed *= SKILL_EFFECT_BUFF_FRONZEN_SPEED_RATIO;

		GetEffectPlaneComponent()->SetVisibility(true);
		GetEffectPlaneComponent()->SetMaterial(0, FrozenMaterialBody);

		UE_LOG(LogSkillComponent, Verbose, TEXT("%s, %s, Using SkillBuff:[%d], OldWalkSpeed:[%f], NewWalkSpeed:[%f]"),
			*SpatialWorkerId, *FString(__FUNCTION__), SkillEffect.EffectId, OldWalkSpeed, MovementComponent->MaxJogSpeed);

		if (!GetWorldWrapper()->GetGameInstance()->IsDedicatedServerInstance())
		{
			FVector SkillLocation = SkillEffect.Causer->GetActorLocation();
			FRotator SkillRotation = SkillEffect.Causer->GetActorRotation();
			AActor* Storm = GetWorldWrapper()->SpawnActor<AActor>(GameState->StormClass,
				SkillLocation,
				SkillRotation);

			UProjectileMovementComponent* ProjectileMovementComponent =
				Cast<UProjectileMovementComponent>(Storm->GetComponentByClass(UProjectileMovementComponent::StaticClass()));

			ProjectileMovementComponent->HomingTargetComponent = CapsuleComponent;
			ProjectileMovementComponent->bIsHomingProjectile = true;
			ProjectileMovementComponent->HomingAccelerationMagnitude = 25000;

			SkillEffect.ClientEffectActors.Add(Storm);
		}
	}
	break;

	case SkillEffect_Buff_Poisonous:
	{
		if (!GetWorldWrapper()->GetGameInstance()->IsDedicatedServerInstance())
		{
			FVector SkillLocation = SkillEffect.Causer->GetActorLocation();
			FRotator SkillRotation = SkillEffect.Causer->GetActorRotation();
			AActor* Poison = GetWorldWrapper()->SpawnActor<AActor>(GameState->PoisonClass,
				SkillLocation,
				SkillRotation);

			UProjectileMovementComponent* ProjectileMovementComponent =
				Cast<UProjectileMovementComponent>(Poison->GetComponentByClass(UProjectileMovementComponent::StaticClass()));

			ProjectileMovementComponent->HomingTargetComponent = CapsuleComponent;
			ProjectileMovementComponent->bIsHomingProjectile = true;
			ProjectileMovementComponent->HomingAccelerationMagnitude = 25000;

			SkillEffect.ClientEffectActors.Add(Poison);
		}

		GetEffectPlaneComponent()->SetVisibility(true);
		GetEffectPlaneComponent()->SetMaterial(0, PoisonousMaterialBody);
	}
	break;

	default:
	{
	}
	}

	SkillEffectTriggerEvent.Broadcast(SkillEffect);
}

void USkillEffectComponent::UpdateEffect(FSkillEffectStatus& SkillEffect)
{
	AC10KGameState* GameState = Cast<AC10KGameState>(GetWorldWrapper()->GetGameState());
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();
	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());
	UGDKMovementComponent* MovementComponent = Cast<UGDKMovementComponent>(OwnerCharacter->GetCharacterMovement());
	int64 NowTs = GameState->NowTs;

	UE_LOG(LogSkillComponent, Verbose, TEXT("%s, %s, Using SkillEffect:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillEffect.EffectId);

	switch (SkillEffect.EffectId)
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
		FDamageEvent De;
		OwnerCharacter->TakeDamageCrossServer(3, De, OwnerCharacter->GetController(), OwnerCharacter);
	}
	break;

	case SKillEffect_Buff_Frozen:
	{
	}
	break;

	case SkillEffect_Buff_Poisonous:
	{
		FDamageEvent De;
		OwnerCharacter->TakeDamageCrossServer(1, De, OwnerCharacter->GetController(), OwnerCharacter);
	}
	break;

	default:
	{
	}
	}

	SkillEffectUpdateEvent.Broadcast(SkillEffect);
}

void USkillEffectComponent::ClearEffect(FSkillEffectStatus& SkillEffect)
{
	AC10KGameState* GameState = Cast<AC10KGameState>(GetWorldWrapper()->GetGameState());
	const FString SpatialWorkerId = GetWorldWrapper()->GetGameInstance()->GetSpatialWorkerId();
	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());
	UGDKMovementComponent* MovementComponent = Cast<UGDKMovementComponent>(OwnerCharacter->GetCharacterMovement());
	int64 NowTs = GameState->NowTs;

	UE_LOG(LogSkillComponent, Verbose, TEXT("%s, %s, Using SkillEffect:[%d]"), *SpatialWorkerId, *FString(__FUNCTION__), SkillEffect.EffectId);

	switch (SkillEffect.EffectId)
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

		UE_LOG(LogSkillComponent, Verbose, TEXT("%s, %s, Using SkillEffect:[%d], OldWalkSpeed:[%f], NewWalkSpeed:[%f]"),
			*SpatialWorkerId, *FString(__FUNCTION__), SkillEffect.EffectId, OldWalkSpeed, MovementComponent->MaxJogSpeed);
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

	GetEffectPlaneComponent()->SetVisibility(false);

	SkillEffectClearEvent.Broadcast(SkillEffect);
}

bool USkillEffectComponent::HasEffects()
{
	for (int32 Idx = 0; Idx < SkillEffect_Max; ++Idx)
	{
		if (EffectStatus[Idx].EffectId)
		{
			return true;
		}
	}

	return false;
}

UStaticMeshComponent* USkillEffectComponent::GetEffectPlaneComponent()
{
	if (EffectPlaneComponent)
	{
		return EffectPlaneComponent;
	}

	AGDKCharacter* OwnerCharacter = Cast<AGDKCharacter>(GetOwner());
	TArray<UActorComponent*> Components = OwnerCharacter->GetComponentsByTag(UStaticMeshComponent::StaticClass(), FName(TEXT("EffectPlane")));
	EffectPlaneComponent = Cast<UStaticMeshComponent>(Components[0]);
	return EffectPlaneComponent;
}
