// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Actor.h"
#include "Runtime/AIModule/Classes/GenericTeamAgentInterface.h"
#include "TimerManager.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFloatValue, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FDamageTakenEvent, float, Value, FVector, Source, FVector, Impact, int32, InstigatorPlayerId, FGenericTeamId, InstigatorTeamId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeathCauserEvent, const AController*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDamageCauserEvent, const AController*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDeathEvent);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	virtual void TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable)
	bool GrantShield(float Value);

	UFUNCTION(BlueprintCallable)
	bool GrantHealth(float Value);

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetCurrentHealth() const
	{
		return CurrentHealth;
	}

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMaxHealth() const
	{
		return MaxHealth;
	}

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetCurrentArmour() const
	{
		return CurrentArmour;
	}

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMaxArmour() const
	{
		return MaxArmour;
	}

	UPROPERTY(BlueprintAssignable)
	FFloatValue HealthUpdated;

	UPROPERTY(BlueprintAssignable)
	FFloatValue ArmourUpdated;

	UPROPERTY(BlueprintAssignable)
	FDamageTakenEvent DamageTaken;

	UPROPERTY(BlueprintAssignable)
	FDeathCauserEvent AuthoritativeDeath;

	UPROPERTY(BlueprintAssignable)
	FDamageCauserEvent AuthoritativeDamage;

	UPROPERTY(BlueprintAssignable)
	FDeathEvent Death;

protected:
	// Notifies all clients that a the character has been hit and from what direction.
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDamageTaken(float Value, FVector Source, FVector Impact, int32 InstigatorPlayerId, FGenericTeamId InstigatorTeamId);

	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION()
	void OnRep_CurrentArmour();

	// Max health this character can have.
	UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "1"))
	float MaxHealth;

	// Current health of the character, can be at most MaxHealth.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CurrentHealth, Category = "Health")
	float CurrentHealth;

	// Max armour this character can have.
	UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "1"))
	float MaxArmour;

	// Current armour of the character, can be at most MaxArmour.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CurrentArmour, Category = "Health")
	float CurrentArmour;

	FTimerHandle HealthRegenerationHandle;

	UFUNCTION()
	void RegenerateHealth();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthRegenValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthRegenCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthRegenInterval;

	FTimerHandle ArmourRegenerationHandle;

	UFUNCTION()
	void RegenerateArmour();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArmourRegenValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArmourRegenCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArmourRegenInterval;
	
	// When hit by radial damage, we assume the impact point is the owner's actor location plus this value
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector RadialDamageImpactOffset;
};
