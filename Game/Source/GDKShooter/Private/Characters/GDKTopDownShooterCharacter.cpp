// Fill out your copyright notice in the Description page of Project Settings.

#include "GDKTopDownShooterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GDKLogging.h"
#include "Components/CapsuleComponent.h"

AGDKTopDownShooterCharacter::AGDKTopDownShooterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BaseTurnRate = 0;
	BaseLookUpRate = 0;

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
		float offsetYaw = TopDownCamera->GetComponentRotation().Yaw;
		if (bMinus) offsetYaw -= this->GetControlRotation().Yaw;
		if (bAdd) offsetYaw += this->GetControlRotation().Yaw;
		auto CameraYaw = FRotator(0, offsetYaw, 0);
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
		float offsetYaw = TopDownCamera->GetComponentRotation().Yaw;
		if (bMinus) offsetYaw -= this->GetControlRotation().Yaw;
		if (bAdd) offsetYaw += this->GetControlRotation().Yaw;
		auto CameraYaw = FRotator(0, offsetYaw, 0);
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
	bool AnyMouseMovement = MouseMovement.X != 0 && MouseMovement.Y != 0;
	bool SprintMovement = IsSprinting() && GetVelocity().SizeSquared() > 0.1f;
	if (AnyMouseMovement || SprintMovement)
	{
		float targetAngle = 0;
		float strength = 1;
		if (SprintMovement)
		{
			targetAngle = FMath::RadiansToDegrees(GetVelocity().HeadingAngle());
		}
		else
		{
			targetAngle = FMath::RadiansToDegrees(MouseMovement.HeadingAngle());
			targetAngle -= TopDownCamera->GetComponentRotation().Yaw;
			targetAngle -= 90;
			targetAngle = -targetAngle;
			strength = MouseMovement.Size();
		}
		targetAngle = ClampAngle(targetAngle);
		float currentAngle = this->GetControlRotation().Yaw;
		currentAngle = ClampAngle(currentAngle);
		float delta = targetAngle - currentAngle;
		delta = ClampAngle(delta);
		float RotationSpeed = MouseRotationSpeed;
		Super::AddControllerYawInput(delta * strength * MouseRotationSpeed); 

	}
	TopDownCamera->SetWorldLocation(this->GetActorLocation() + InitialCameraOffset);
	MouseMovement = FVector::ZeroVector;
}
