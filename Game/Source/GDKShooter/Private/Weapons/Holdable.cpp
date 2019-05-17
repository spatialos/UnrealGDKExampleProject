// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Holdable.h"
#include "GDKLogging.h"
#include "Components/EquippedComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "UnrealNetwork.h"


AHoldable::AHoldable()
{
 	// Default to not ticking
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bReplicateMovement = true;

	LocationComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(LocationComponent);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	Mesh->SetupAttachment(RootComponent);
}

void AHoldable::BeginPlay()
{
	Super::BeginPlay();
	OnMetaDataUpdated();
}

void AHoldable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHoldable, MetaData);
	DOREPLIFETIME(AHoldable, CurrentMode);
}

void AHoldable::StartPrimaryUse_Implementation() { IsPrimaryUsing = true; }
void AHoldable::StopPrimaryUse_Implementation() { IsPrimaryUsing = false; }
void AHoldable::StartSecondaryUse_Implementation() { IsSecondaryUsing = true; }
void AHoldable::StopSecondaryUse_Implementation() { IsSecondaryUsing = false; }

void AHoldable::OnRep_MetaData()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	OnMetaDataUpdated();
}

void AHoldable::SetMetaData(const FGDKMetaData& NewMetaData)
{
	if (HasAuthority())
	{
		MetaData = NewMetaData;
	}
	OnMetaDataUpdated();
}

void AHoldable::SetIsActive_Implementation(bool bNewIsActive)
{
	bIsActive = bNewIsActive;
	this->SetActorHiddenInGame(!bNewIsActive);
	StopPrimaryUse();
	StopSecondaryUse();
}

FVector AHoldable::EffectSpawnPoint()
{
	return Mesh->GetSocketLocation(EffectSocketName);
}

void AHoldable::SetFirstPerson_Implementation(bool bNewFirstPerson)
{
	bIsFirstPerson = bNewFirstPerson;
	Mesh->CastShadow = !bNewFirstPerson;
}

void AHoldable::ToggleMode_Implementation()
{
	bIsFirstPerson = bNewFirstPerson;
	Mesh->CastShadow = !bNewFirstPerson;
}
