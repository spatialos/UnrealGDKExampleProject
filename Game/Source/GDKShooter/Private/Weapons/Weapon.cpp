// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Weapon.h"

#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"
#include "GDKLogging.h"
#include "UnrealNetwork.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	bDrawDebugLineTrace = false;
	MaxRange = 50000.0f;

	BufferShotThreshold = 0.2f;
}


void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->DebugDrawTraceTag = kTraceTag;
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

FVector AWeapon::GetLineTraceDirection()
{
	if (!GetShootingComponent())
	{
		return FVector::ZeroVector;
	}

	return GetShootingComponent()->GetLineTraceDirection();
}

void AWeapon::AnnounceShot(bool bHit)
{
	if (GetShootingComponent())
	{
		GetShootingComponent()->FireShot(this);
	}
}

void AWeapon::DoFire_Implementation()
{

}

FInstantHitInfo AWeapon::DoLineTrace()
{
	FInstantHitInfo OutHitInfo;

	if (!GetShootingComponent())
	{
		UE_LOG(LogGDK, Verbose, TEXT("Weapon %s does not have a shooting component"), *this->GetName());
		return OutHitInfo;
	}

	FCollisionQueryParams TraceParams;
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(this);
	TraceParams.AddIgnoredActor(GetOwner());

	if (bDrawDebugLineTrace)
	{
		TraceParams.TraceTag = kTraceTag;
	}

	FHitResult HitResult(ForceInit);
	FVector TraceStart = GetShootingComponent()->GetLineTraceStart();
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
		return OutHitInfo;
	}

	OutHitInfo.Location = HitResult.ImpactPoint;
	OutHitInfo.HitActor = HitResult.GetActor();

	OutHitInfo.bDidHit = true;

	return OutHitInfo;
}

void AWeapon::StartPrimaryUse_Implementation()
{
	Super::StartPrimaryUse_Implementation();

	bHasBufferedShot = true;
	BufferedShotUntil = UGameplayStatics::GetRealTimeSeconds(GetWorld()) + BufferShotThreshold;

	if (GetMovementComponent())
	{
		GetMovementComponent()->SetIsBusy(true);
	}
}

void AWeapon::StopPrimaryUse_Implementation()
{
	Super::StopPrimaryUse_Implementation();

	if (!HasBufferedShot())
	{
		if (GetMovementComponent())
		{
			GetMovementComponent()->SetIsBusy(false);
		}
	}
}

void AWeapon::StartSecondaryUse_Implementation()
{
	Super::StartSecondaryUse_Implementation();

	if (GetMovementComponent())
	{
		GetMovementComponent()->SetAiming(true);
		GetMovementComponent()->SetAimingRotationModifier(AimingRotationSpeed);
	}
}

void AWeapon::StopSecondaryUse_Implementation()
{
	Super::StopSecondaryUse_Implementation();

	if (GetMovementComponent())
	{
		GetMovementComponent()->SetAiming(false);
	}
}

void AWeapon::ForceCooldown(float Cooldown)
{
	Super::ForceCooldown(Cooldown);
	NextShotTime = FMath::Max(NextShotTime, UGameplayStatics::GetRealTimeSeconds(GetWorld()) + Cooldown);

}

bool AWeapon::ReadyToFire()
{
	float Now = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	return Now > NextShotTime;
}

bool AWeapon::BufferedShotStillValid()
{
	return UGameplayStatics::GetRealTimeSeconds(GetWorld()) < BufferedShotUntil;
}

bool AWeapon::HasBufferedShot()
{
	return bHasBufferedShot;
}

void AWeapon::ConsumeBufferedShot()
{
	BufferedShotUntil = 0;
	bHasBufferedShot = false;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if ((IsPrimaryUsing || HasBufferedShot()) && ReadyToFire())
	{
		ConsumeBufferedShot();
		DoFire();

		if (!IsPrimaryUsing)
		{
			if (GetMovementComponent())
			{
				GetMovementComponent()->SetIsBusy(false);
			}
		}
	}

	if (HasBufferedShot() && !BufferedShotStillValid())
	{
		ConsumeBufferedShot();
		if (!IsPrimaryUsing)
		{
			if (GetMovementComponent())
			{
				GetMovementComponent()->SetIsBusy(false);
			}
		}
	}
}

void AWeapon::SetIsActive(bool bNewIsActive)
{
	Super::SetIsActive(bNewIsActive);

	ConsumeBufferedShot();
}

void AWeapon::RefreshComponentCache()
{
	CachedOwner = GetOwner();
	if (CachedOwner == nullptr)
	{
		CachedMovementComponent = nullptr;
		CachedShootingComponent = nullptr;
	}
	else
	{
		CachedMovementComponent = Cast<UGDKMovementComponent>(CachedOwner->GetComponentByClass(UGDKMovementComponent::StaticClass()));
		CachedShootingComponent = Cast<UShootingComponent>(CachedOwner->GetComponentByClass(UShootingComponent::StaticClass()));
	}
}

UGDKMovementComponent* AWeapon::GetMovementComponent()
{
	if (GetOwner() != CachedOwner)
	{
		RefreshComponentCache();
	}
	return CachedMovementComponent;
}

UShootingComponent* AWeapon::GetShootingComponent()
{
	if (!GetOwner())
	{
		RefreshComponentCache();
	}
	return CachedShootingComponent;
}
