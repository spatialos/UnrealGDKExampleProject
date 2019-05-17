// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKFPShooterCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/FirstPersonTraceProvider.h"
#include "GDKLogging.h"
#include "UnrealNetwork.h"

FName AGDKFPShooterCharacter::FirstPersonMeshComponentName(TEXT("FirstPersonMesh"));

AGDKFPShooterCharacter::AGDKFPShooterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	// Hide the ThirdPerson Mesh from Owner
	GetMesh()->bOwnerNoSee = true;
	GetMesh()->bOnlyOwnerSee = false;

	// Create the first person camera
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent()); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FirstPersonCamera->bUsePawnControlRotation = true;
	
	// Create the FirstPerson Mesh
	FirstPersonMesh = CreateOptionalDefaultSubobject<USkeletalMeshComponent>(AGDKFPShooterCharacter::FirstPersonMeshComponentName);
	if (FirstPersonMesh)
	{
		FirstPersonMesh->AlwaysLoadOnClient = true;
		FirstPersonMesh->AlwaysLoadOnServer = false;
		FirstPersonMesh->bOwnerNoSee = false;
		FirstPersonMesh->bOnlyOwnerSee = true;
		FirstPersonMesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPose;
		FirstPersonMesh->bCastDynamicShadow = false;
		FirstPersonMesh->CastShadow = false;
		FirstPersonMesh->bAffectDynamicIndirectLighting = true;
		FirstPersonMesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		FirstPersonMesh->SetupAttachment(FirstPersonCamera);
		static FName MeshCollisionProfileName(TEXT("CharacterMesh"));
		FirstPersonMesh->SetCollisionProfileName(MeshCollisionProfileName);
		FirstPersonMesh->SetGenerateOverlapEvents(false);
		FirstPersonMesh->SetCanEverAffectNavigation(false);
	}

	UFirstPersonTraceProvider* TraceProvider = CreateDefaultSubobject<UFirstPersonTraceProvider>(TEXT("TraceProvider"));

}

FVector AGDKFPShooterCharacter::GetLineTraceStart() const
{
	return FirstPersonCamera->GetComponentLocation();
}

FVector AGDKFPShooterCharacter::GetLineTraceDirection() const
{
	return FirstPersonCamera->GetForwardVector();
}

void AGDKFPShooterCharacter::AttachHoldable(AHoldable* Holdable, FName Socket) const
{
	if (IsLocallyControlled())
	{
		Holdable->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
		Holdable->SetFirstPerson(true);
	}
	else
	{
		Super::AttachHoldable(Holdable, Socket);
	}
}
