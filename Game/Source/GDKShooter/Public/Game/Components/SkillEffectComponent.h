// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillEffectComponent.generated.h"

#define SKILL_EFFECT_BUFF_FRONZEN_SPEED_RATIO		0.3

DECLARE_LOG_CATEGORY_EXTERN(LogSkillComponent, Log, All);

class AGDKCharacter;

UENUM(BlueprintType)
enum ESkillId
{
	SkillId_None = 0,
	SkillId_FireBall,
	SkillId_Storm,
	SkillId_Poison,
};

UENUM(BlueprintType)
enum ESkillType
{
	SkillType_None = 0,
	SkillType_SingleTarget,
	SkillType_MultipleTargets,
	SkillType_CircleArea,
};

UENUM(BlueprintType)
enum ESkillEffect
{
	SkillEffect_None = 0,
	SkillEffect_Instant_Min,
	SkillEffect_Instant_FireBall,
	SkillEffect_Instant_Storm,
	SkillEffect_Instant_Poison,
	SkillEffect_Instant_Max,

	SkillEffect_Buff_Min,
	SKillEffect_Buff_Firing,
	SKillEffect_Buff_Frozen,
	SkillEffect_Buff_Poisonous,
	SkillEffect_Buff_Max,

	SkillEffect_Max,
};

USTRUCT(BlueprintType)
struct FSkillEffectDesc
{
	GENERATED_USTRUCT_BODY();

	int32				SkillEffectId = SkillEffect_None;

	// yunjie: don't set this field to zero, since it would be reset immediately in the next frame then the client will not receive 
	// this change as the status on server side changed back
	int32				SkillEffectTime = 0;
};

USTRUCT(BlueprintType)
struct FSkillDesc
{
	GENERATED_USTRUCT_BODY();

	int32						id = SkillId_None;
	ESkillType					SkillType = SkillType_None;
	TArray<FSkillEffectDesc>	SkillEffects;

	int32					Probability = 0;

	bool IsValid() const
	{
		return id != SkillId_None;
	}
};

USTRUCT(BlueprintType)
struct FSkillEffectStatus
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(Category = "SkillEffect", EditAnywhere, BlueprintReadWrite)
	AGDKCharacter		*Causer = nullptr;
	UPROPERTY(Category = "SkillEffect", EditAnywhere, BlueprintReadWrite)
	int32				EffectId = 0;
	UPROPERTY(Category = "SkillEffect", EditAnywhere, BlueprintReadWrite)
	int64				ExpireTime = 0;
	UPROPERTY(Category = "SkillEffect", EditAnywhere, BlueprintReadWrite)
	int32				ExecuteCount = 0;

public:
	bool				bTriggered = false;
	TArray<AActor*>		ClientEffectActors;

	void Reset()
	{
		Causer = nullptr;
		EffectId = 0;
		ExpireTime = 0;
		ExecuteCount = 0;
		bTriggered = false;

		for (auto ClientEffectActor : ClientEffectActors)
		{
			ClientEffectActor->Destroy();
		}
		ClientEffectActors.Reset();
	}

	bool IsInstant() const
	{
		return EffectId > SkillEffect_Instant_Min && EffectId < SkillEffect_Instant_Max;
	}

	bool IsBuff() const
	{
		return EffectId > SkillEffect_Buff_Min && EffectId < SkillEffect_Buff_Max;
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
			// return EffectId != 0 && NowTs < ExpireTime && !bTriggered;
			return EffectId != 0 && !bTriggered;
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
			// return EffectId != 0 && NowTs < ExpireTime && bTriggered;
			return EffectId != 0 && bTriggered;
		}

		return false;
	}

	bool ShouldBeCleared(int64 NowTs) const
	{
		return IsExpired(NowTs) && bTriggered;

		if (IsInstant())
		{
			// return EffectId != 0 && bTriggered;
		}

		if (IsBuff())
		{
			// return EffectId != 0 && NowTs >= ExpireTime && bTriggered;
		}

		return false;
	}

	bool IsExpired(int64 NowTs) const
	{
		return EffectId != 0 && NowTs >= ExpireTime;
	}

	bool IsEmpty() const
	{
		return EffectId == 0 && ExpireTime == 0 && !Causer && !bTriggered;
	}

	bool SanityCheck() const
	{
		return Causer != nullptr;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSkillEffectTriggerEvent, const FSkillEffectStatus&, SkillEffectStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSkillEffectUpdateEvent, const FSkillEffectStatus&, SkillEffectStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSkillEffectClearEvent, const FSkillEffectStatus&, SkillEffectStatus);

UCLASS()
class GDKSHOOTER_API USkillEffectComponent: public UActorComponent
{
	GENERATED_BODY()

public:
	USkillEffectComponent();

	// yunjie: fundamental interfaces BEGIN
	UFUNCTION(BlueprintCallable)
	void UseSkillOnServer(int32 SkillId, TArray<AGDKCharacter*> Targets);
	void ProcessSkillEffectOnServer(const FSkillEffectDesc& SkillEffect, TArray<AGDKCharacter*> Targets);
	UFUNCTION()
	void ProcessEffectTimer();
	void UpdateEffectStatus();
	void TriggerEffect(FSkillEffectStatus& SkillEffect);
	void UpdateEffect(FSkillEffectStatus& SkillEffect);
	void ClearEffect(FSkillEffectStatus& SkillEffect);

	bool HasEffects();
	// yunjie: fundamental interfaces END 

	// yunjie: interfaces oriented to blueprint BEGIN
	UFUNCTION(BlueprintCallable)
	void UseSkillRandomly();
	// yunjie: interfaces oriented to blueprint END 

	// yunjie: Called on client side BEGIN
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastUseSkill(int32 SkillId, const TArray<AGDKCharacter*>& Targets);
	void ProcessSkillEffectOnClient(const FSkillEffectDesc& SkillEffect, const TArray<AGDKCharacter*>& Targets);

	UFUNCTION()
	void OnRep_EffectStatus();

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

	UStaticMeshComponent* GetEffectPlaneComponent();

public:
	UPROPERTY(Category = SkillComponent, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32									TargetsRadius = 1000;

	UPROPERTY(ReplicatedUsing = OnRep_EffectStatus, EditAnywhere, Category = SkillComponent)
	FSkillEffectStatus								EffectStatus[SkillEffect_Max];

	UPROPERTY(EditDefaultsOnly, Category = SkillComponent)
	UMaterialInterface* DefaultMaterialBody;

	UPROPERTY(EditDefaultsOnly, Category = SkillComponent)
	UMaterialInterface* FiringMaterialBody;

	UPROPERTY(EditDefaultsOnly, Category = SkillComponent)
	UMaterialInterface* FrozenMaterialBody;

	UPROPERTY(EditDefaultsOnly, Category = SkillComponent)
	UMaterialInterface* PoisonousMaterialBody;

	UStaticMeshComponent* EffectPlaneComponent;

	UPROPERTY(BlueprintAssignable)
		FSkillEffectTriggerEvent SkillEffectTriggerEvent;

	UPROPERTY(BlueprintAssignable)
		FSkillEffectUpdateEvent SkillEffectUpdateEvent;

	UPROPERTY(BlueprintAssignable)
		FSkillEffectClearEvent SkillEffectClearEvent;
};
