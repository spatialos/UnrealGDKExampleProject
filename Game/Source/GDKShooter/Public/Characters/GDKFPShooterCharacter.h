// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Core/GDKEquippedCharacter.h"
#include "GDKFPShooterCharacter.generated.h"

/**
 * 
 */
UCLASS()
class GDKSHOOTER_API AGDKFPShooterCharacter : public AGDKEquippedCharacter
{
	GENERATED_BODY()

public:
	AGDKFPShooterCharacter(const FObjectInitializer& ObjectInitializer);
	
	// Returns a position from which to start a line trace.
	// Use this so your line trace doesn't collide with the player character.
	virtual FVector GetLineTraceStart() const;

	// Returns the direction in which to perform a line trace so it lines up with the center of the crosshair.
	virtual FVector GetLineTraceDirection() const;

	virtual void AttachHoldable(AHoldable* Holdable, FName Socket) const override;

protected:

	/** Name of the FirstPersonMeshComponent. Use this name if you want to prevent creation of the component (with ObjectInitializer.DoNotCreateDefaultSubobject). */
	static FName FirstPersonMeshComponentName;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FirstPersonCamera;

	/** The first person skeletal mesh associated with this Character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* FirstPersonMesh;
		
};
