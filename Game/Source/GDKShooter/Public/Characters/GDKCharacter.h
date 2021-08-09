// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInstance.h"
#include "GameFramework/Character.h"
#include "Characters/Components/HealthComponent.h"
#include "Characters/Components/EquippedComponent.h"
#include "Characters/Components/MetaDataComponent.h"
#include "Characters/Components/GDKMovementComponent.h"
#include "Characters/Components/TeamComponent.h"
#include "Weapons/Holdable.h"
#include "TimerManager.h"
#include "Runtime/AIModule/Classes/GenericTeamAgentInterface.h"
#include "Runtime/AIModule/Classes/Perception/AISightTargetInterface.h"
#include "GDKCharacter.generated.h"

DECLARE_DELEGATE_OneParam(FBoolean, bool);
DECLARE_DELEGATE_OneParam(FHoldableSelection, int32);

UCLASS()
class GDKSHOOTER_API AGDKCharacter : public ACharacter, public IGenericTeamAgentInterface, public IAISightTargetInterface
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

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UTeamComponent* TeamComponent;

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

	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual bool CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor = NULL) const override;

	UPROPERTY(EditDefaultsOnly)
	TArray<FName> LineOfSightSockets;

	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<ECollisionChannel> LineOfSightCollisionChannel;

	UPROPERTY(EditAnywhere)
	float RagdollLifetime = 5.0f;

	// [client + server] Puts the player in ragdoll mode.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StartRagdoll();

	// ----- HACKY WORKAROUND, KEEP AWAY FROM PRODUCTION CODE ----
	// This class is used by simplayer, which locally replaces the Pawn's controller without the server's knowledge
	// Unpossessing a character resets its movement data, so this is used to inform the server that it happened.
	// Getting rid of this hack means adding AIController-like capabilities to the regular PlayerController.
	UFUNCTION(BlueprintCallable, Server, Unreliable)
	void ClientMovementReset();

	UFUNCTION(BlueprintCallable)
	void LookAround();

	UFUNCTION(BlueprintCallable)
	void StopLookingAround();

	UFUNCTION(BlueprintPure)
	bool IsLookingAround();

private:
	UFUNCTION()
	void DeleteSelf();

	UFUNCTION()
	void OnTakeDamage(AActor* Target, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	FTimerHandle DeletionTimer;
	FTimerDelegate DeletionDelegate;

	bool bLookingAround;
};
