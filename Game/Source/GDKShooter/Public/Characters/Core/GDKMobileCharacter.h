// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GDKCharacter.h"
#include "Components/GDKMovementComponent.h"
#include "GDKMobileCharacter.generated.h"

DECLARE_EVENT_TwoParams(AGDKMobileCharacter, FAimingChanged, bool, float);
DECLARE_DELEGATE_OneParam(FBoolean, bool);
/**
 * 
 */
UCLASS()
class GDKSHOOTER_API AGDKMobileCharacter : public AGDKCharacter
{
	GENERATED_BODY()

public:
	AGDKMobileCharacter(const FObjectInitializer& ObjectInitializer);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	// [server + client] Returns true if the character is currently sprinting.
	UFUNCTION(BlueprintPure, Category = "Movement")
		bool IsSprinting();
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UGDKMovementComponent* GDKMovementComponent;

	/** Handles moving forward/backward */
	virtual void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	virtual void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	virtual void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	virtual void LookUpAtRate(float Rate);
};
