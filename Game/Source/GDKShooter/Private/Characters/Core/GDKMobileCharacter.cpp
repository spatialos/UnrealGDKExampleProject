// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKMobileCharacter.h"
#include "GDKLogging.h"
#include "UnrealNetwork.h"

#include "Components/GDKMovementComponent.h"

AGDKMobileCharacter::AGDKMobileCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGDKMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	
	// Configure character movement
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

// Called every frame
void AGDKMobileCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bJumpedThisFrame = false;
	if (HasAuthority())
	{
		Pitch = GetControlRotation().Pitch;
	}
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

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AGDKMobileCharacter::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AGDKMobileCharacter::StopSprinting);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AGDKMobileCharacter::StartCrouching);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AGDKMobileCharacter::StopCrouching);

	/*
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AGDKCharacter::Interact);
	PlayerInputComponent->BindAction("DebugResetCharacter", IE_Pressed, this, &AGDKCharacter::DebugResetCharacter);
	*/
}

void AGDKMobileCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGDKMobileCharacter, Pitch);
}

void AGDKMobileCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AGDKMobileCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AGDKMobileCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGDKMobileCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AGDKMobileCharacter::StartSprinting()
{
	if (UGDKMovementComponent* MovementComponent = Cast<UGDKMovementComponent>(GetCharacterMovement()))
	{
		MovementComponent->SetWantsToSprint(true);
	}
}

void AGDKMobileCharacter::StopSprinting()
{
	if (UGDKMovementComponent* MovementComponent = Cast<UGDKMovementComponent>(GetCharacterMovement()))
	{
		MovementComponent->SetWantsToSprint(false);
	}
}

void AGDKMobileCharacter::StartCrouching()
{
	Crouch(true);
}

void AGDKMobileCharacter::StopCrouching()
{
	UnCrouch(true);
}

bool AGDKMobileCharacter::HasSprintedRecently()
{
	if (UGDKMovementComponent* MovementComponent = Cast<UGDKMovementComponent>(GetCharacterMovement()))
	{
		return MovementComponent->HasSprintedRecently();
	}
	return IsSprinting();
}

bool AGDKMobileCharacter::IsSprinting()
{
	UGDKMovementComponent* Movement = Cast<UGDKMovementComponent>(GetCharacterMovement());
	if (Movement == nullptr)
	{
		return false;
	}

	if (Role >= ROLE_AutonomousProxy)
	{
		// If we're authoritative or the owning client, we know definitively whether we're sprinting.
		return Movement->IsSprinting();
	}

	// For all other client types, we need to guess based on speed.
	// Add a tolerance factor to the max jog speed and use that as a sprint threshold.
	float SquaredSprintThreshold = Movement->MaxWalkSpeed + 10.0f;
	SquaredSprintThreshold *= SquaredSprintThreshold;

	// We only care about speed in the X-Y plane.
	return GetVelocity().SizeSquared2D() > SquaredSprintThreshold;
}

void AGDKMobileCharacter::OnJumped_Implementation()
{
	bJumpedThisFrame = true;
}

bool AGDKMobileCharacter::JumpedThisFrame()
{
	return bJumpedThisFrame;
}

