// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInstance.h"
#include "GameFramework/Character.h"
#include "Components/HealthComponent.h"
#include "Components/EquippedComponent.h"
#include "Components/MetaDataComponent.h"
#include "Components/GDKMovementComponent.h"
#include "Weapons/Holdable.h"
#include "TimerManager.h"
#include "GDKCharacter.generated.h"

DECLARE_DELEGATE_OneParam(FBoolean, bool);
DECLARE_DELEGATE_OneParam(FHoldableSelection, int32);

UCLASS()
class GDKSHOOTER_API AGDKCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGDKCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UHealthComponent* HealthComponent;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UGDKMovementComponent* GDKMovementComponent;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UEquippedComponent* EquippedComponent;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UMetaDataComponent* MetaDataComponent;
	
	// [server] Tells this player that it's time to die.
	// @param Killer  The player who killed me. Can be null if it wasn't a player who dealt the damage that killed me.
	UFUNCTION()
		virtual void Die(const class AActor* Killer);

	UFUNCTION(BlueprintPure)
		float GetRemotePitch() {
			return RemoteViewPitch;
		}

	/** Handles moving forward/backward */
	virtual void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	virtual void MoveRight(float Val);

	UFUNCTION(BlueprintNativeEvent)
		void OnEquippedUpdated(AHoldable* NewHoldable);

private:
	
	virtual void TornOff() override;

	// [client + server] Puts the player in ragdoll mode.
	UFUNCTION()
		void StartRagdoll();

	UFUNCTION()
		void DeleteSelf();

	FTimerHandle DeletionTimer;
	FTimerDelegate DeletionDelegate;
	
public:

	float TakeDamage(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(CrossServer, Reliable)
		void TakeDamageCrossServer(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
};
