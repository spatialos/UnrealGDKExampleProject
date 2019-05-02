// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Game/GDKMetaData.h"
#include "Materials/MaterialInstance.h"
#include "GameFramework/Character.h"
#include "GDKCharacter.generated.h"

USTRUCT(BlueprintType)
struct FGDKCharacterMaterials
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UMaterialInstance* CharacterMaterial;
};

UCLASS()
class GDKSHOOTER_API AGDKCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGDKCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetMetaData(FGDKMetaData MetaData);

	UFUNCTION(BlueprintCallable)
		void IgnoreMe(AActor* ToIgnore);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Destroyed() override;

protected:

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

	// If true, the character is currently ragdoll-ing.
	UPROPERTY(ReplicatedUsing = OnRep_IsRagdoll)
		bool bIsRagdoll;

	// If true, the character is currently ragdoll-ing.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MetaData)
		FGDKMetaData MetaData;

	// [server] Tells this player that it's time to die.
	// @param Killer  The player who killed me. Can be null if it wasn't a player who dealt the damage that killed me.
	virtual void Die(const class AGDKCharacter* Killer);

	UFUNCTION()
		virtual void OnRep_MetaData();

	UFUNCTION(BlueprintImplementableEvent)
		void OnMetaDataUpdated();

	FTimerHandle RegenerationHandle;

	UFUNCTION()
	void RegenerateHealth();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float HealthRegenValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float HealthRegenCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float HealthRegenInterval;


private:
	
	// [client + server] Puts the player in ragdoll mode.
	void StartRagdoll();

	// [owning client + server] Updates the aim variables to be sync-ed out to clients, or updates the values locally
	// if we're executing on the owning client.
	// Will only update the angles if they differ from the current stored value by more than AngleUpdateThreshold.
	void UpdateAimRotation(float AngleUpdateThreshold);

	// Notifies all clients that a the character has been hit and from what direction.
	UFUNCTION(NetMulticast, Unreliable)
		void MulticastDamageTaken(FVector DamageSource);

	UFUNCTION()
		void OnRep_CurrentHealth();

	UFUNCTION()
		void OnRep_CurrentArmour();

	UFUNCTION()
		void OnRep_IsRagdoll();

	UFUNCTION()
		void DeleteSelf();

	FTimerHandle DeletionTimer;
	FTimerDelegate DeletionDelegate;
	
public:

	UFUNCTION(BlueprintCallable)
		bool GrantShield(float Value);

	float TakeDamage(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(CrossServer, Reliable)
		void TakeDamageCrossServer(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	UPROPERTY(Replicated)
	int32 PlayerId;

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
	
	// Returns the player's name, as specified on login.
	FString GetPlayerName() const;

	FORCEINLINE FGDKMetaData GetMetaData() const
	{
		return MetaData;
	}

	// Called on clients when the player health changes
	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
		void OnHealthUpdated(float NewHealth, float MaximumHealth);

	// Called on clients when the player armour changes
	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
		void OnArmourUpdated(float NewArmour, float MaximumHealth);

	// Called on clients when the player health changes
	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
		void OnDamageTaken(FVector DamageSource);

	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
		void ShowRespawnScreen();
	
};
