// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Core/GDKEquippedCharacter.h"
#include "GDKTPShooterCharacter.generated.h"

/**
 * 
 */
UCLASS()
class GDKSHOOTER_API AGDKTPShooterCharacter : public AGDKEquippedCharacter
{
	GENERATED_BODY()

public:
	AGDKTPShooterCharacter(const FObjectInitializer& ObjectInitializer);

	// Returns a position from which to start a line trace.
	// Use this so your line trace doesn't collide with the player character.
	virtual FVector GetLineTraceStart() const;

	// Returns the direction in which to perform a line trace so it lines up with the center of the crosshair.
	virtual FVector GetLineTraceDirection() const;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UCameraComponent* GetThirdPersonCamere() const { return ThirdPersonCamera; }

protected:

	/** Third person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* ThirdPersonCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;
	
	
};
