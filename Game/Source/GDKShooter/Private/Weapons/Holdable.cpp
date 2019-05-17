// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Holdable.h"
#include "GDKLogging.h"
#include "Components/EquippedComponent.h"
#include "UnrealNetwork.h"


AHoldable::AHoldable()
{
 	// Default to not ticking
	PrimaryActorTick.bCanEverTick = false;

	LocationComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(LocationComponent);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	Mesh->SetupAttachment(RootComponent);
}

void AHoldable::BeginPlay()
{
	Super::BeginPlay();
	OnRep_Wielder();
	OnMetaDataUpdated();
}

void AHoldable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHoldable, Wielder);
	DOREPLIFETIME(AHoldable, MetaData);
	DOREPLIFETIME(AHoldable, CurrentMode);
}

bool AHoldable::AssignTo(UEquippedComponent* NewOwner)
{
	check(HasAuthority());

	Wielder = NewOwner;

	return true;
}

// Wielder has updated, if null then we are (hopefully) a pickup
void AHoldable::OnRep_Wielder()
{
	if (Wielder)
	{
		Wielder->InformWielderOfWielded(this);
	}
	else
	{
		// Deal with being dropped or late checkout of wielder
	}
}

void AHoldable::OnRep_MetaData()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	OnMetaDataUpdated();
}

void AHoldable::SetMetaData(FGDKMetaData NewMetaData)
{
	if (HasAuthority())
	{
		MetaData = NewMetaData;
	}
	OnMetaDataUpdated();
}

void AHoldable::SetIsActive(bool bNewIsActive)
{
	//TODO Find logic for sheathing inactive weapons
	UE_LOG(LogGDK, Error, TEXT("SetIsActive %d on %s"), bNewIsActive, *this->GetName());
	bIsActive = bNewIsActive;
	this->SetActorHiddenInGame(!bNewIsActive);
	StopPrimaryUse();
	StopSecondaryUse();
}

FVector AHoldable::EffectSpawnPoint()
{
	return Mesh->GetSocketLocation(EffectSocketName);
}

void AHoldable::ForceCooldown(float Cooldown)
{

}
