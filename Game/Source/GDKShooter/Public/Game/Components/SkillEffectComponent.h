// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillEffectComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSkillComponent, Log, All);

class AGDKCharacter;

enum ESkillId : int32
{
	SkillId_None = 0,
	SkillId_FireBall,
	SkillId_Storm,
	SkillId_Poison,
};

enum ESkillType : int32
{
	SkillType_None = 0,
	SkillType_SingleTarget,
	SkillType_MultipleTargets,
	SkillType_CircleArea,
};

enum ESkillEffect : int32
{
	SkillEffect_None = 0,
	SkillEffect_Instant_FireBall,
	SkillEffect_Instant_Storm,
	SkillEffect_Instant_Poison,
	SkillEffect_Instant_Max,

	SKillEffect_Buff_Firing,
	SKillEffect_Buff_Frozen,
	SkillEffect_Buff_Poisonous,
	SkillEffect_Buff_Max,

	SkillEffect_Max,
};

#define SKILL_EFFECT_BUFF_FRONZEN_SPEED_RATIO		0.5
#define SKILL_EFFECT_BUFF_TIME						5

struct FSkillDesc
{
	int32					id = SkillId_None;
	ESkillType				SkillType = SkillType_None;
	TArray<ESkillEffect>	SkillEffects;

	int32					Probability = 0;

	bool IsValid() const
	{
		return id != SkillId_None;
	}
};

USTRUCT()
struct FEffectBuffStatus
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY()
	int32				EffectId = 0;
	UPROPERTY()
	int64				ExpireTime = 0;
	UPROPERTY()
	int32				ExecuteCount = 0;

	bool				bTriggered = false;

	void Reset()
	{
		EffectId = 0;
		ExpireTime = 0;
		ExecuteCount = 0;
		bTriggered = false;
	}

	bool IsInstant() const
	{
		return EffectId < SkillEffect_Instant_Max;
	}

	bool IsBuff() const
	{
		return EffectId < SkillEffect_Buff_Max;
	}

	void Trigger()
	{
		bTriggered = true;
	}

	bool IsTriggered() const { return bTriggered; }

	bool ShouldBeTriggered(int64 NowTs) const
	{
		if (IsInstant())
		{
			return EffectId != 0 && !bTriggered;
		}

		if (IsBuff())
		{
			return EffectId != 0 && NowTs < ExpireTime && !bTriggered;
		}

		return false;
	}

	bool AlreadyBeTriggered(int64 NowTs) const
	{
		if (IsInstant())
		{
			return EffectId != 0 && bTriggered;
		}

		if (IsBuff())
		{
			return EffectId != 0 && NowTs < ExpireTime && bTriggered;
		}

		return false;
	}

	bool ShouldBeCleared(int64 NowTs) const
	{
		if (IsInstant())
		{
			return EffectId != 0 && bTriggered;
		}

		if (IsBuff())
		{
			return EffectId != 0 && NowTs >= ExpireTime && bTriggered;
		}

		return false;
	}

	bool IsValid(int64 NowTs) const
	{
		return EffectId != 0;
	}
};

UCLASS()
class GDKSHOOTER_API USkillEffectComponent: public UActorComponent
{
	GENERATED_BODY()

public:
	USkillEffectComponent();

	// yunjie: fundamental interfaces BEGIN
	UFUNCTION(BlueprintCallable)
	void UseSkillOnServer(int32 SkillId, TArray<AGDKCharacter*> Targets);
	void ProcessSkillEffectOnServer(int32 SkillEffect, TArray<AGDKCharacter*> Targets);
	UFUNCTION()
	void ProcessEffectTimer();
	void UpdateEffectStatus();
	void TriggerEffect(int32 SkillEffect);
	void UpdateEffect(int32 SkillEffect);
	void ClearEffect(int32 SkillEffect);

	// yunjie: fundamental interfaces END 

	// yunjie: interfaces oriented to blueprint BEGIN
	UFUNCTION(BlueprintCallable)
	void UseSkillRandomly();
	// yunjie: interfaces oriented to blueprint END 

	// yunjie: Called on client side BEGIN
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastUseSkill(int32 SkillId, const TArray<AGDKCharacter*>& Targets);
	void ProcessSkillEffectOnClient(int32 SkillEffect, const TArray<AGDKCharacter*>& Targets);

	UFUNCTION()
	void OnRep_EffectStatus();

	UFUNCTION()
	void OnRep_aaa();
	// yunjie: Called on client side END

protected:

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}

	TMap<int32, FSkillDesc>					SkillTable;
	FTimerHandle							EffectTimer;

	UWorld* GetWorldWrapper() { return GWorld; }

public:
	UPROPERTY(Category = SkillComponent, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32									TargetsRadius = 1000;

	UPROPERTY(ReplicatedUsing = OnRep_EffectStatus, EditAnywhere, Category = "Skill")
	FEffectBuffStatus								EffectStatus[SkillEffect_Max];

	UPROPERTY(ReplicatedUsing = OnRep_aaa, EditAnywhere, Category = "Skill")
	int aaa;
};
