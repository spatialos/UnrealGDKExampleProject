// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKFPShooterCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GDKLogging.h"

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
		FirstPersonMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
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
}

FVector AGDKFPShooterCharacter::GetLineTraceStart() const
{
	return FirstPersonCamera->GetComponentLocation();
}

FVector AGDKFPShooterCharacter::GetLineTraceDirection() const
{
	return FirstPersonCamera->GetForwardVector();
}

void AGDKFPShooterCharacter::AttachWeapon(AWeapon* Weapon) const
{
	if (IsLocallyControlled())
	{
		Weapon->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, kRightGunSocketName);
		UE_LOG(LogGDK, Log, TEXT("AGDKFPShooterCharacter:: Attached %s to %s"), *Weapon->GetName(), *Weapon->GetAttachParentSocketName().ToString());
		Weapon->EnableShadows(false);
		Weapon->SetFirstPerson(true);
	}
	else
	{
		Super::AttachWeapon(Weapon);
	}
}
