// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKMobileCharacter.h"
#include "GDKLogging.h"


AGDKMobileCharacter::AGDKMobileCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGDKMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	
	// Configure character movement
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	GDKMovementComponent = Cast<UGDKMovementComponent>(GetCharacterMovement());
}

// Called to bind functionality to input
void AGDKMobileCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGDKMobileCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGDKMobileCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGDKMobileCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGDKMobileCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction<FBoolean>("Sprint", IE_Pressed, GDKMovementComponent, &UGDKMovementComponent::SetWantsToSprint, true);
	PlayerInputComponent->BindAction<FBoolean>("Sprint", IE_Released, GDKMovementComponent, &UGDKMovementComponent::SetWantsToSprint, false);

	PlayerInputComponent->BindAction<FBoolean>("Crouch", IE_Pressed, this, &AGDKMobileCharacter::Crouch, true);
	PlayerInputComponent->BindAction<FBoolean>("Crouch", IE_Released, this, &AGDKMobileCharacter::UnCrouch, true);
}

void AGDKMobileCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AGDKMobileCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AGDKMobileCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGDKMobileCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AGDKMobileCharacter::IsSprinting()
{
	if (GDKMovementComponent == nullptr)
	{
		return false;
	}

	if (Role >= ROLE_AutonomousProxy)
	{
		// If we're authoritative or the owning client, we know definitively whether we're sprinting.
		return GDKMovementComponent->IsSprinting();
	}

	// For all other client types, we need to guess based on speed.
	// Add a tolerance factor to the max jog speed and use that as a sprint threshold.
	float SquaredSprintThreshold = GDKMovementComponent->MaxWalkSpeed + 10.0f;
	SquaredSprintThreshold *= SquaredSprintThreshold;

	// We only care about speed in the X-Y plane.
	return GetVelocity().SizeSquared2D() > SquaredSprintThreshold;
}

