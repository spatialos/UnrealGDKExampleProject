// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "InstantWeapon.h"

#include "Characters/Core/GDKShooterCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GDKLogging.h"
#include "UnrealNetwork.h"


AInstantWeapon::AInstantWeapon()
{
	BurstInterval = 0.5f;
	BurstCount = 1;
	ShotInterval = 0.2f;
	LastBurstTime = 0.0f;
	BurstShotsRemaining = 0;
	ShotBaseDamage = 10.0f;
	HitValidationTolerance = 50.0f;
	DamageTypeClass = UDamageType::StaticClass();  // generic damage type
	ShotVisualizationDelayTolerance = FTimespan::FromMilliseconds(3000.0f);
}

void AInstantWeapon::ActuallyStartFiring()
{
	SetWeaponState(EWeaponState::Firing);

	// Initialize the burst.
	LastBurstTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	BurstShotsRemaining = BurstCount;
}

bool AInstantWeapon::ReadyToStartFiring()
{
	float Now = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	if (BurstCount > 0)
	{
		return GetWeaponState() == EWeaponState::Idle && Now > LastBurstTime + BurstInterval;
	}
	else
	{
		return GetWeaponState() == EWeaponState::Idle && ReadyToFire();
	}
}

bool AInstantWeapon::ReadyToFire()
{
	float Now = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	return Now > LastShotTime + ShotInterval;
}

void AInstantWeapon::StartFire()
{
	check(GetNetMode() == NM_Client);

	if (!bIsActive)
	{
		return;
	}

	float Now = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	if (ReadyToFire())
	{
		ActuallyStartFiring();
	}
	else
	{
		ShotBuffered = true;
		ShotBufferedUntil = Now + ShotInputLeniancy;
	}
}

void AInstantWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetNetMode() != NM_Client)
	{
		return;
	}

	if (GetWeaponState() == EWeaponState::Firing)
	{
		if (ReadyToFire())
		{
			DoFire();
		}
	}
	else if(ShotBuffered)
	{
		float Now = UGameplayStatics::GetRealTimeSeconds(GetWorld());
		if (ShotBufferedUntil > Now)
		{
			if (ReadyToFire())
			{
				ShotBuffered = false;
				ActuallyStartFiring();
				DoFire();
			}
		}
		else
		{
			ShotBuffered = false;
		}
	}
}

void AInstantWeapon::StopFire()
{
	if (GetNetMode() != NM_Client)
	{
		return;
	}

	ShotBuffered = false;
	// Can't force stop a burst.
	if (GetWeaponState() == EWeaponState::Firing && !IsBurstFire())
	{
		StopFiring();
	}
}

void AInstantWeapon::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->DebugDrawTraceTag = kTraceTag;
}

void AInstantWeapon::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
}

void AInstantWeapon::DoFire()
{
	check(GetNetMode() == NM_Client);

	if (!GetOwningCharacter()->CanFire())
	{
		// Don't attempt to fire if the character can't.
		return;
	}

	LastShotTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());

	FInstantHitInfo HitInfo;
	if (DoLineTrace(HitInfo))
	{
		ServerDidHit(HitInfo);
		SpawnFX(HitInfo, true);  // Spawn the hit fx locally
		AnnounceShot(HitInfo.HitActor->bCanBeDamaged);
	}
	else
	{
		ServerDidMiss(HitInfo);
		SpawnFX(HitInfo, false);  // Spawn the hit fx locally
		AnnounceShot(false);
	}

	if (IsBurstFire())
	{
		--BurstShotsRemaining;
		if (BurstShotsRemaining <= 0)
		{
			FinishedBurst();
			StopFiring();
		}
	}

}

FVector AInstantWeapon::GetLineTraceDirection()
{
	float SpreadToUse = GetOwningCharacter()->IsAiming() ? SpreadAt100mWhenAiming : SpreadAt100m;
	if (SpreadToUse > 0)
	{
		auto Spread = FMath::RandPointInCircle(SpreadToUse);
		return GetOwningCharacter()->GetLineTraceDirection().Rotation().RotateVector(FVector(10000, Spread.X, Spread.Y));
	}
	else
	{
		return GetOwningCharacter()->GetLineTraceDirection();
	}
}

void AInstantWeapon::NotifyClientsOfHit(const FInstantHitInfo& HitInfo, bool bImpact)
{
	check(GetNetMode() < NM_Client);

	MulticastNotifyHit(HitInfo, bImpact);
}

void AInstantWeapon::SpawnFX(const FInstantHitInfo& HitInfo, bool bImpact)
{
	if (GetNetMode() < NM_Client)
	{
		return;
	}
	
	AInstantWeapon::OnRenderShot(HitInfo.Location, bImpact);
}

bool AInstantWeapon::ValidateHit(const FInstantHitInfo& HitInfo)
{
	check(GetNetMode() < NM_Client);

	if (HitInfo.HitActor == nullptr)
	{
		return false;
	}

	// Get the bounding box of the actor we hit.
	const FBox HitBox = HitInfo.HitActor->GetComponentsBoundingBox();

	// Calculate the extent of the box along all 3 axes an add a tolerance factor.
	FVector BoxExtent = 0.5 * (HitBox.Max - HitBox.Min) + (HitValidationTolerance * FVector::OneVector);
	FVector BoxCenter = (HitBox.Max + HitBox.Min) * 0.5;

	// Avoid precision errors for really thin objects.
	BoxExtent.X = FMath::Max(20.0f, BoxExtent.X);
	BoxExtent.Y = FMath::Max(20.0f, BoxExtent.Y);
	BoxExtent.Z = FMath::Max(20.0f, BoxExtent.Z);

	// Check whether the hit is within the box + tolerance.
	if (FMath::Abs(HitInfo.Location.X - BoxCenter.X) > BoxExtent.X ||
		FMath::Abs(HitInfo.Location.Y - BoxCenter.Y) > BoxExtent.Y ||
		FMath::Abs(HitInfo.Location.Z - BoxCenter.Z) > BoxExtent.Z)
	{
		return false;
	}

	return true;
}

void AInstantWeapon::DealDamage(const FInstantHitInfo& HitInfo)
{
	FPointDamageEvent DmgEvent;
	DmgEvent.DamageTypeClass = DamageTypeClass;
	DmgEvent.HitInfo.ImpactPoint = HitInfo.Location;

	HitInfo.HitActor->TakeDamage(ShotBaseDamage, DmgEvent, GetOwningCharacter()->GetController(), this);
}

bool AInstantWeapon::ServerDidHit_Validate(const FInstantHitInfo& HitInfo)
{
	return true;
}

void AInstantWeapon::ServerDidHit_Implementation(const FInstantHitInfo& HitInfo)
{
	if (!GetOwningCharacter()->CanFire())
	{
		UE_LOG(LogGDK, Verbose, TEXT("%s server: rejected shot because character is unable to fire"), *this->GetName());
		return;
	}

	bool bDoNotifyHit = false;

	if (HitInfo.HitActor == nullptr)
	{
		bDoNotifyHit = true;
	}
	else
	{
		if (ValidateHit(HitInfo))
		{
			DealDamage(HitInfo);
			bDoNotifyHit = true;
		}
		else
		{
			UE_LOG(LogGDK, Verbose, TEXT("%s server: rejected hit of actor %s"), *this->GetName(), *HitInfo.HitActor->GetName());
		}
	}

	if (bDoNotifyHit)
	{
		NotifyClientsOfHit(HitInfo, true);
	}
}

bool AInstantWeapon::ServerDidMiss_Validate(const FInstantHitInfo& HitInfo)
{
	return true;
}

void AInstantWeapon::ServerDidMiss_Implementation(const FInstantHitInfo& HitInfo)
{
	if (!GetOwningCharacter()->CanFire())
	{
		UE_LOG(LogGDK, Verbose, TEXT("%s server: rejected shot because character is unable to fire"), *this->GetName());
		return;
	}

	NotifyClientsOfHit(HitInfo, false);
}

void AInstantWeapon::StopFiring()
{
	check(GetNetMode() == NM_Client);
	ShotBuffered = false;
	SetWeaponState(EWeaponState::Idle);
}

void AInstantWeapon::MulticastNotifyHit_Implementation(FInstantHitInfo HitInfo, bool bImpact)
{
	// Make sure we're a client, and we're not the client that owns this gun (they will have already played the effect locally).
	if (GetNetMode() != NM_DedicatedServer &&
		(GetOwningCharacter() == nullptr || !GetOwningCharacter()->IsLocallyControlled()))
	{
		SpawnFX(HitInfo, bImpact);
	}
}
