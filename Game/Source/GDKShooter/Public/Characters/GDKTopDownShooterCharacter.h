// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GDKCharacter.h"
#include "GDKTopDownShooterCharacter.generated.h"

UCLASS()
class GDKSHOOTER_API AGDKTopDownShooterCharacter : public AGDKCharacter
{
	GENERATED_BODY()

public:
	AGDKTopDownShooterCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Returns a position from which to start a line trace.
	// Use this so your line trace doesn't collide with the player character.
	virtual FVector GetLineTraceStart() const;

	// Returns the direction in which to perform a line trace so it lines up with the center of the crosshair.
	virtual FVector GetLineTraceDirection() const;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* TopDownCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MouseRotationSpeed = 1;

	virtual void MoveForwardRight(float Val);
	virtual void MoveForward(float Val) override;
	virtual void MoveRight(float Val) override;

	virtual void MouseHorizontal(float Val);
	virtual void MouseVertical(float Val);

	virtual void AddControllerPitchInput(float Val) override;
	virtual void AddControllerYawInput(float Val) override;
	
	FVector MouseMovement;
	FVector InitialCameraOffset;
	float InitialMeshRotationOffset;
	bool bRotationToApply;

	bool bMinus;
	bool bAdd;
};
