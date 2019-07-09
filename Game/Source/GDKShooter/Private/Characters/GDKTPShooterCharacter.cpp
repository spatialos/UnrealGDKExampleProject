// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKTPShooterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

AGDKTPShooterCharacter::AGDKTPShooterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create the third person camera
	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	ThirdPersonCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

FVector AGDKTPShooterCharacter::GetLineTraceStart() const
{
	return EquippedComponent->CurrentlyHeldItem()->EffectSpawnPoint();
}

FVector AGDKTPShooterCharacter::GetLineTraceDirection() const
{
	return ThirdPersonCamera->GetForwardVector();
}
