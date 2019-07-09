// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKFPShooterCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/FirstPersonTraceProvider.h"
#include "Components/SkeletalMeshComponent.h"
#include "GDKLogging.h"
#include "Components/SkinnedMeshComponent.h"

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

	CreateDefaultSubobject<UFirstPersonTraceProvider>(TEXT("TraceProvider"));
}

void AGDKFPShooterCharacter::OnEquippedUpdated_Implementation(AHoldable* Holdable)
{
	if (Holdable != nullptr)
	{
		USkeletalMeshComponent* MeshToAttachTo = IsLocallyControlled() ? FirstPersonMesh : GetMesh();
		Holdable->AttachToComponent(MeshToAttachTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Holdable->GetActiveSocket());
		Holdable->SetFirstPerson(IsLocallyControlled());
	}
}
