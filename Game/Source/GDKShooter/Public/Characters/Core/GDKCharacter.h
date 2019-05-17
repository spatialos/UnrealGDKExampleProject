// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInstance.h"
#include "GameFramework/Character.h"
#include "Components/HealthComponent.h"
#include "GDKCharacter.generated.h"

UCLASS()
class GDKSHOOTER_API AGDKCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGDKCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UHealthComponent* HealthComponent;
	
	// [server] Tells this player that it's time to die.
	// @param Killer  The player who killed me. Can be null if it wasn't a player who dealt the damage that killed me.
	UFUNCTION()
		virtual void Die(const class AActor* Killer);

	// If true, the character is currently ragdoll-ing.
	UPROPERTY(ReplicatedUsing = OnRep_IsRagdoll)
		bool bIsRagdoll;

	UFUNCTION(BlueprintPure)
		float GetRemotePitch() {
			return RemoteViewPitch;
		}

private:
	// [client + server] Puts the player in ragdoll mode.
	UFUNCTION()
		void StartRagdoll();

	UFUNCTION()
		void OnRep_IsRagdoll();

	UFUNCTION()
		void DeleteSelf();

	FTimerHandle DeletionTimer;
	FTimerDelegate DeletionDelegate;
	
public:

	float TakeDamage(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(CrossServer, Reliable)
		void TakeDamageCrossServer(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
};
