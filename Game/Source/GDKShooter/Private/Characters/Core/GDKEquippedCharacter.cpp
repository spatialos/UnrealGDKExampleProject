// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKEquippedCharacter.h"
#include "GDKLogging.h"
#include "UnrealNetwork.h"

#include "Controllers/GDKPlayerController.h"
#include "Weapons/Holdable.h"

AGDKEquippedCharacter::AGDKEquippedCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EquippedComponent = CreateDefaultSubobject<UEquippedComponent>(TEXT("Equipment"));
	MetaDataComponent = CreateDefaultSubobject<UMetaDataComponent>(TEXT("MetaData"));
}

void AGDKEquippedCharacter::BeginPlay()
{
	Super::BeginPlay();

	EquippedComponent->HoldableUpdated.AddDynamic(this, &AGDKEquippedCharacter::OnEquippedUpdated);
	EquippedComponent->BusyUpdated.AddDynamic(GDKMovementComponent, &UGDKMovementComponent::SetIsBusy);
	MetaDataComponent->MetaDataUpdated.AddDynamic(EquippedComponent, &UEquippedComponent::SpawnStarterTemplates);
}

void AGDKEquippedCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Primary", IE_Pressed, this, &AGDKEquippedCharacter::StartPrimaryUse);
	PlayerInputComponent->BindAction("Primary", IE_Released, EquippedComponent, &UEquippedComponent::StopPrimaryUse);
	PlayerInputComponent->BindAction("Secondary", IE_Pressed, EquippedComponent, &UEquippedComponent::StartSecondaryUse);
	PlayerInputComponent->BindAction("Secondary", IE_Released, EquippedComponent, &UEquippedComponent::StopSecondaryUse);

	PlayerInputComponent->BindAction< FHoldableSelection>("1", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 0);
	PlayerInputComponent->BindAction< FHoldableSelection>("2", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 1);
	PlayerInputComponent->BindAction< FHoldableSelection>("3", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 2);
	PlayerInputComponent->BindAction< FHoldableSelection>("4", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 3);
	PlayerInputComponent->BindAction< FHoldableSelection>("5", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 4);
	PlayerInputComponent->BindAction< FHoldableSelection>("6", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 5);
	PlayerInputComponent->BindAction< FHoldableSelection>("7", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 6);
	PlayerInputComponent->BindAction< FHoldableSelection>("8", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 7);
	PlayerInputComponent->BindAction< FHoldableSelection>("9", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 8);
	PlayerInputComponent->BindAction< FHoldableSelection>("0", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 9);
	PlayerInputComponent->BindAction< FHoldableSelection>("-", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 10);
	PlayerInputComponent->BindAction< FHoldableSelection>("+", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 11);
}

void AGDKEquippedCharacter::StartPrimaryUse()
{

	if (IsSprinting())
	{
		// Don't allow sprinting and use at the same time.
		EquippedComponent->ForceCooldown(SprintRecoveryTime);
	}

	EquippedComponent->StartPrimaryUse();
}

void AGDKEquippedCharacter::OnEquippedUpdated(AHoldable* NewHoldable)
{
	if (NewHoldable)
	{
		AttachHoldable(NewHoldable, NewHoldable->GetActiveSocket());
	}
	OnHeldChanged();
}

void AGDKEquippedCharacter::AttachHoldable(AHoldable* Holdable, FName Socket) const
{
	Holdable->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
}
