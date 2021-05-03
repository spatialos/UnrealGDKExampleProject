// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillComponent.generated.h"

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
	SkillEffect_FireBall,
	SkillEffect_Storm,
	SkillEffect_Poison,
};

enum ESkillBuff : int32
{
	SkillBuff_None = 0,
	SKillBuff_Firing,
	SKillBuff_Frozen,
	SkillBuff_Poisonous,
};

struct FSkillDesc
{
	int32					id = 0;
	ESkillType				SkillType = SkillType_None;
	TArray<ESkillEffect>	SkillEffects;
	TArray<ESkillBuff>		SkillBuffs;
};

UCLASS()
class GDKSHOOTER_API USkillComponent: public UActorComponent
{
	GENERATED_BODY()

public:
	USkillComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void UseSkill(int32 SkillId, TArray<AGDKCharacter*> Targets);
	UFUNCTION()
	void ProcessSkillEffect(int32 SkillEffect, TArray<AGDKCharacter*> Targets);
	UFUNCTION()
	void ProcessSkillBuff(int32 SkillBuff, TArray<AGDKCharacter*> Targets);

protected:
	void BeginPlay();

	TMap<int32, FSkillDesc>					SkillTable;
};
