// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFloatValue, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVectorEvent, FVector, Source);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeathEvent, const AActor*, Causer);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable)
		bool GrantShield(float Value);

	UFUNCTION(BlueprintCallable)
		bool GrantHealth(float Value);

	FORCEINLINE float GetCurrentHealth() const
	{
		return CurrentHealth;
	}

	FORCEINLINE float GetMaxHealth() const
	{
		return MaxHealth;
	}

	FORCEINLINE float GetCurrentArmour() const
	{
		return CurrentArmour;
	}

	FORCEINLINE float GetMaxArmour() const
	{
		return MaxArmour;
	}

	UPROPERTY(BlueprintAssignable)
		FFloatValue HealthUpdated;
	UPROPERTY(BlueprintAssignable)
		FFloatValue ArmourUpdated;
	UPROPERTY(BlueprintAssignable)
		FVectorEvent DamageTaken;
	UPROPERTY(BlueprintAssignable)
		FDeathEvent Death;

protected:

	// Notifies all clients that a the character has been hit and from what direction.
	UFUNCTION(NetMulticast, Unreliable)
		void MulticastDamageTaken(FVector DamageSource);

	UFUNCTION()
		void OnRep_CurrentHealth();

	UFUNCTION()
		void OnRep_CurrentArmour();

	// Max health this character can have.
	UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "1"))
		int32 MaxHealth;

	// Current health of the character, can be at most MaxHealth.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CurrentHealth, Category = "Health")
		int32 CurrentHealth;

	// Max armour this character can have.
	UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "1"))
		int32 MaxArmour;

	// Current armour of the character, can be at most MaxArmour.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CurrentArmour, Category = "Health")
		int32 CurrentArmour;

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
		
	
};
