// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Weapon.h"

#include "Characters/Core/GDKShooterCharacter.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "CollisionQueryParams.h"
#include "GDKLogging.h"
#include "UnrealNetwork.h"


AWeapon::AWeapon()
	: CurrentState(EWeaponState::Idle)
	, OwningCharacter(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	
	bReplicates = true;
	bReplicateMovement = true;

	bDrawDebugLineTrace = false;
	MaxRange = 50000.0f;

	LocationComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(LocationComponent);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	Mesh->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (!bHasAttached)
	{
		TryToAttach();
	}

	OnMetaDataUpdated();
}

void AWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bHasAttached)
	{
		TryToAttach();
	}
}

void AWeapon::StopFire() {}

class AGDKShooterCharacter* AWeapon::GetOwningCharacter() const
{
	return OwningCharacter;
}

void AWeapon::SetOwningCharacter(AGDKShooterCharacter* NewCharacter)
{
	OwningCharacter = NewCharacter;
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, OwningCharacter);
	DOREPLIFETIME(AWeapon, MetaData);
	DOREPLIFETIME(AWeapon, bIsActive);
}

FVector AWeapon::GetLineTraceDirection()
{
	return GetOwningCharacter()->GetLineTraceDirection();
}

EWeaponState AWeapon::GetWeaponState() const
{
	return CurrentState;
}

void AWeapon::SetWeaponState(EWeaponState NewState)
{
	CurrentState = NewState;
}

void AWeapon::OnRep_MetaData()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	OnMetaDataUpdated();
}

void AWeapon::SetMetaData(FGDKMetaData NewMetaData)
{
	if (HasAuthority())
	{
		MetaData = NewMetaData;
	}
	OnMetaDataUpdated();
}

void AWeapon::EnableShadows(bool bShadows)
{
	Mesh->CastShadow = bShadows;
}

void AWeapon::SetFirstPerson(bool bFirstPerson)
{
	this->bFirstPerson = bFirstPerson;
}

bool AWeapon::IsFirstPerson()
{
	return bFirstPerson;
}

void AWeapon::TryToAttach()
{
	if (GetRootComponent()->GetAttachParent()
		&& GetRootComponent()->GetAttachParent()->GetOwner())
	{
		AGDKShooterCharacter* ShooterCharacter = Cast<AGDKShooterCharacter>(GetRootComponent()->GetAttachParent()->GetOwner());
		if (ShooterCharacter) {
			ShooterCharacter->AttachWeapon(this);
			bHasAttached = true;
		}
	}
	this->SetActorHiddenInGame(!bIsActive);
}

void AWeapon::OnRep_IsActive()
{
	StopFire();
	this->SetActorHiddenInGame(!bIsActive);
}

void AWeapon::AddShotListener(FShotDelegate Listener)
{
	ShotCallback = Listener;
}
void AWeapon::RemoveShotListener()
{
	ShotCallback.Unbind();
}

void AWeapon::AnnounceShot(bool bHit)
{
	ShotCallback.ExecuteIfBound(bHit);
}

FVector AWeapon::BulletSpawnPoint()
{
	return Mesh->GetSocketLocation(BarrelSocketName);
}

bool AWeapon::DoLineTrace(FInstantHitInfo& OutHitInfo)
{
	AGDKShooterCharacter* Character = GetOwningCharacter();
	if (Character == nullptr)
	{
		UE_LOG(LogGDK, Verbose, TEXT("Weapon %s does not have an owning character"), *this->GetName());
		return false;
	}

	FCollisionQueryParams TraceParams;
	TraceParams.bTraceComplex = true;
	//TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(this);
	TraceParams.AddIgnoredActor(Character);

	if (bDrawDebugLineTrace)
	{
		TraceParams.TraceTag = kTraceTag;
	}

	FHitResult HitResult(ForceInit);
	FVector TraceStart = Character->GetLineTraceStart();
	FVector TraceEnd = TraceStart + GetLineTraceDirection() * MaxRange;

	bool bDidHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		TraceChannel,
		TraceParams);

	if (!bDidHit)
	{
		OutHitInfo.Location = TraceEnd;
		return false;
	}

	OutHitInfo.Location = HitResult.ImpactPoint;
	OutHitInfo.HitActor = HitResult.GetActor();

	return true;
}
