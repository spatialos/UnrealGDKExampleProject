// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GDKMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimingUpdated, bool, bIsAiming);
UCLASS()
class GDKSHOOTER_API UGDKMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	friend class FSavedMove_GDKMovement;

	UGDKMovementComponent(const FObjectInitializer& ObjectInitializer);

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	
	// Sets whether the character is trying to sprint.
	void SetWantsToSprint(bool bSprinting);

	// Returns true if the character is actually sprinting.
	UFUNCTION(BlueprintPure, Category = "Sprint")
		bool IsSprinting() const;

	// Set whether the character is allowed to sprint
	UFUNCTION(BlueprintCallable, Category = "Sprint")
		void SetSprintEnabled(bool bSprintEnabled);

	// Returns true if the character is actually sprinting.
	UFUNCTION(BlueprintPure, Category = "Sprint")
		bool HasSprintedRecently() const;
	
	// Set if the character should be aiming
	UFUNCTION(Server, Reliable, WithValidation)
		void SetAiming(bool NewValue);

	// Returns true if the character is aiming.
	UFUNCTION(BlueprintPure, Category = "Aiming")
		bool IsAiming() const;

	UPROPERTY(BlueprintAssignable, Category = "Aiming")
		FAimingUpdated OnAimingUpdated;

	void SetAimingRotationModifier(float NewAimingRotationModifier) { AimingRotationModifier = NewAimingRotationModifier; }
	UFUNCTION(BlueprintPure, Category = "Aiming")
		float GetAimingRotationModifier() { return AimingRotationModifier; }

	// Returns the max speed of the character, modified if sprinting.
	virtual float GetMaxSpeed() const override;

	// Returns the max acceleration of the character, modified if sprinting.
	virtual float GetMaxAcceleration() const override;

	// True if movement direction is within SprintDirectionTolerance of the look direction.
	UFUNCTION(BlueprintPure)
		bool IsMovingForward() const;

	// Time in miliseconds since IsSprinting was last true
	double TimeSinceLastSprint() const;

		// Multiply max speed by this factor when sprinting.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Character Movement: Walking")
		float MaxJogSpeed;

	/** Overriding this method to stop crouching when falling */
	virtual bool CanCrouchInCurrentState() const override;

	// Set if the character is busy
	UFUNCTION(BlueprintCallable)
		void SetIsBusy(bool bBusy) { bIsBusy = bBusy; }

	// Set if the character is busy
	UFUNCTION(BlueprintPure)
		bool IsBusy() const { return bIsBusy; }

private:

	// Override whether sprint is allowed.
	uint8 bCanSprint : 1;

	// If true, the player is attempting to sprint. The character will sprint if all conditions are met
	// (e.g. the player is moving in a direction within SprintDirectionTolerance of the camera direction).
	uint8 bWantsToSprint : 1;

	// Set to true on each frame that IsSprinting is true, for detecting when sprinting stops
	uint8 bWasSprintingLastFrame : 1;

	// Last time, in miliseconds, when sprinting was true
	FDateTime lastTimeSprinting;

	// If true, the player is aiming, should therefore move slower, and should not be allowed to sprint.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_IsAiming)
		bool bIsAiming;

	UFUNCTION()
		void OnRep_IsAiming();

	float AimingRotationModifier = 1;

	// If true, the player is using something, e.g. Shooting, so should not sprint
	uint8 bIsBusy : 1;

	// If true, the player will attempt to rotate all the way to the control rotation. Used to correct for
	// over-rotation while standing still (e.g. trying have an aim offset of > 90 degrees).
	uint8 bShouldOrientToControlRotation : 1;

	// Multiply max speed by this factor when sprinting.
	UPROPERTY(EditAnywhere, Category = "Character Movement: Walking")
		float MaxSprintSpeed;

	// Multiply acceleration by this factor when sprinting.
	UPROPERTY(EditAnywhere, Category = "Character Movement (General Settings)")
		float SprintAcceleration;

	// Tolerance between forward and move direction within which the character can sprint.
	// Represents the minimum magnitude of the dot product between the vectors.
	UPROPERTY(EditAnywhere, Category = "Character Movement (General Settings)")
		float SprintDirectionTolerance;

	// Time in miliseconds after sprinting until character has recovered from the sprint.
	// For example to return to a non-sprinting animation before shooting.
	UPROPERTY(EditAnywhere, Category = "Character Movement (General Settings)")
		float SprintCooldown;

	// Multiply acceleration by this factor when aiming.
	UPROPERTY(EditAnywhere, Category = "Character Movement (General Settings)")
		float JogAcceleration;
};

class FSavedMove_GDKMovement : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	virtual void Clear() override;

	virtual uint8 GetCompressedFlags() const override;

	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InPawn, float MaxDelta) const override;

	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;

private:
	uint8 bSavedWantsToSprint : 1;
};

class FNetworkPredictionData_Client_GDKMovement : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_GDKMovement(const UCharacterMovementComponent& ClientMovement);

	typedef FNetworkPredictionData_Client_Character Super;

	virtual FSavedMovePtr AllocateNewMove() override;
};
