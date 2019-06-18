// Fill out your copyright notice in the Description page of Project Settings.

#include "GDKTopDownShooterCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GDKLogging.h"v

AGDKTopDownShooterCharacter::AGDKTopDownShooterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create the top down camera
	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(GetCapsuleComponent());
	TopDownCamera->bUsePawnControlRotation = false;
	TopDownCamera->SetRelativeLocation(FVector(-3000, -3000, 3000));
	TopDownCamera->SetRelativeRotation(FRotator(-35, 45, 0));
	TopDownCamera->SetFieldOfView(20);
}

FVector AGDKTopDownShooterCharacter::GetLineTraceStart() const
{
	return EquippedComponent->CurrentlyHeldItem()->EffectSpawnPoint();
}

FVector AGDKTopDownShooterCharacter::GetLineTraceDirection() const
{
	return GetActorForwardVector();
}

// Called to bind functionality to input
void AGDKTopDownShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForwardRight", this, &AGDKTopDownShooterCharacter::MoveForwardRight);

	PlayerInputComponent->BindAxis("MouseHorizontal", this, &AGDKTopDownShooterCharacter::MouseHorizontal);
	PlayerInputComponent->BindAxis("MouseVertical", this, &AGDKTopDownShooterCharacter::MouseVertical);
}

void AGDKTopDownShooterCharacter::MoveForwardRight(float Value)
{
	if (Value != 0.0f)
	{
		float OffsetYaw = TopDownCamera->GetComponentRotation().Yaw;
		if (bMinus) OffsetYaw -= this->GetControlRotation().Yaw;
		if (bAdd) OffsetYaw += this->GetControlRotation().Yaw;
		auto CameraYaw = FRotator(0, OffsetYaw, 0);
		auto ForwardVector = CameraYaw.RotateVector(FVector::ForwardVector);
		auto RightVector = CameraYaw.RotateVector(FVector::RightVector);
		AddMovementInput(ForwardVector, FMath::Abs(Value));
		AddMovementInput(RightVector, Value);
	}
}

void AGDKTopDownShooterCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		float OffsetYaw = TopDownCamera->GetComponentRotation().Yaw;
		if (bMinus) OffsetYaw -= this->GetControlRotation().Yaw;
		if (bAdd) OffsetYaw += this->GetControlRotation().Yaw;
		auto CameraYaw = FRotator(0, OffsetYaw, 0);
		auto ForwardVector = CameraYaw.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, Value);
	}
}

void AGDKTopDownShooterCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		float offsetYaw = TopDownCamera->GetComponentRotation().Yaw;
		if (bMinus) offsetYaw -= this->GetControlRotation().Yaw;
		if (bAdd) offsetYaw += this->GetControlRotation().Yaw;
		auto CameraYaw = FRotator(0, offsetYaw, 0);
		auto RightVector = CameraYaw.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
}

void AGDKTopDownShooterCharacter::AddControllerPitchInput(float Val)
{
}

void AGDKTopDownShooterCharacter::AddControllerYawInput(float Val)
{
}

void AGDKTopDownShooterCharacter::MouseHorizontal(float Value)
{
	MouseMovement.X = Value;
}
void AGDKTopDownShooterCharacter::MouseVertical(float Value)
{
	MouseMovement.Y = Value;
}
void AGDKTopDownShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitialMeshRotationOffset = GetMesh()->GetRelativeTransform().GetRotation().Rotator().Yaw;
	InitialCameraOffset = TopDownCamera->GetRelativeTransform().GetLocation();
	TopDownCamera->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepRelative, EDetachmentRule::KeepWorld, false));
}

float ClampAngle(float Input)
{
	while (Input < -180)
	{
		Input += 360;
	}
	while (Input > 180)
	{
		Input -= 360;
	}
	return Input;
}

void AGDKTopDownShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	bool bAnyMouseMovement = MouseMovement.X != 0.f && MouseMovement.Y != 0.f;
	bool SprintMovement = GDKMovementComponent->IsSprinting() && GetVelocity().SizeSquared() > 0.1f;
	if (bAnyMouseMovement || SprintMovement)
	{
		float TargetAngle = 0.f;
		float Strength = 1.f;
		if (SprintMovement)
		{
			TargetAngle = FMath::RadiansToDegrees(GetVelocity().HeadingAngle());
		}
		else
		{
			TargetAngle = FMath::RadiansToDegrees(MouseMovement.HeadingAngle());
			TargetAngle -= TopDownCamera->GetComponentRotation().Yaw;
			TargetAngle -= 90;
			TargetAngle = -TargetAngle;
			Strength = MouseMovement.Size();
		}
		TargetAngle = ClampAngle(TargetAngle);
		float currentAngle = this->GetControlRotation().Yaw;
		currentAngle = ClampAngle(currentAngle);
		float delta = TargetAngle - currentAngle;
		delta = ClampAngle(delta);
		float RotationSpeed = MouseRotationSpeed;
		Super::AddControllerYawInput(delta * Strength * MouseRotationSpeed);

	}
	TopDownCamera->SetWorldLocation(this->GetActorLocation() + InitialCameraOffset);
	MouseMovement = FVector::ZeroVector;
}
