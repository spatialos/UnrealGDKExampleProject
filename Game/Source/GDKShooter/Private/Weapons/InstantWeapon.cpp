// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Weapons/InstantWeapon.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GDKLogging.h"
#include "Net/UnrealNetwork.h"

#include "BlastRuntime/Public/BlastMeshActor.h"
#include "BlastRuntime/Public/TestBlastMesh/TestBlastMeshActor.h"

AInstantWeapon::AInstantWeapon()
{
	BurstInterval = 0.5f;
	BurstCount = 1;
	ShotInterval = 0.2f;
	NextBurstTime = 0.0f;
	BurstShotsRemaining = 0;
	ShotBaseDamage = 10.0f;
	HitValidationTolerance = 50.0f;
	DamageTypeClass = UDamageType::StaticClass();  // generic damage type
	ShotVisualizationDelayTolerance = FTimespan::FromMilliseconds(3000.0f);
}

void AInstantWeapon::StartPrimaryUse_Implementation()
{
	if (IsBurstFire())
	{
		if (!IsPrimaryUsing && NextBurstTime < UGameplayStatics::GetRealTimeSeconds(GetWorld()))
		{
			BurstShotsRemaining = BurstCount;
			NextBurstTime = UGameplayStatics::GetRealTimeSeconds(GetWorld()) + BurstInterval;
		}
	}

	Super::StartPrimaryUse_Implementation();
}

void AInstantWeapon::StopPrimaryUse_Implementation()
{
	// Can't force stop a burst.
	if (!IsBurstFire() || bAllowContinuousBurstFire)
	{
		Super::StopPrimaryUse_Implementation();
	}
}

void AInstantWeapon::DoFire_Implementation()
{
	bool bFlag = GetGameInstance()->IsDedicatedServerInstance();
	FString serverString = bFlag ? "SERVER" : "CLIENT";
	UE_LOG(LogGDK, Warning, TEXT("AInstantWeapon::DoFire_Implementation, %s"), *serverString);

	if (!bIsActive)
	{
		IsPrimaryUsing = false;
		ConsumeBufferedShot();
		return;
	}

	NextShotTime = UGameplayStatics::GetRealTimeSeconds(GetWorld()) + ShotInterval;
	
	FInstantHitInfo HitInfo = DoLineTrace();
	if (HitInfo.bDidHit)
	{
		ServerDidHit(HitInfo);
		SpawnFX(HitInfo, true);  // Spawn the hit fx locally
		AnnounceShot(HitInfo.HitActor ? HitInfo.HitActor->CanBeDamaged() : false);

		UE_LOG(LogGDK, Warning, TEXT("AInstantWeapon::DoFire_Implementation 11111111111, %s"), *serverString);

		REAL_BLAST_MESH_ACTOR* blastActor = Cast<REAL_BLAST_MESH_ACTOR>(HitInfo.HitActor);
		UE_LOG(LogGDK, Warning, TEXT("AInstantWeapon::DoFire_Implementation 22222222222222222222222, %s"), *serverString);

		if (blastActor)
		{
			UE_LOG(LogGDK, Warning, TEXT("AInstantWeapon::DoFire_Implementation 33333333333333333333, %s"), *serverString);
			REAL_BLAST_MESH_COMPONENT* blastComp = Cast<REAL_BLAST_MESH_COMPONENT>(blastActor->GetBlastMeshComponent());
			if (blastComp)
			{
				UE_LOG(LogGDK, Warning, TEXT("AInstantWeapon::DoFire_Implementation 44444444444444444444444, %s"), *serverString);
				blastComp->ApplyRadialDamage(HitInfo.Location, 80, 200, 500, 1000);
			}
		}
	}
	else
	{
		ServerDidMiss(HitInfo);
		SpawnFX(HitInfo, false);  // Spawn the hit fx locally
		AnnounceShot(false);

		UE_LOG(LogGDK, Warning, TEXT("AInstantWeapon::DoFire_Implementation 2222222222222, %s"), *serverString);
	}

	if (IsBurstFire())
	{
		--BurstShotsRemaining;
		if (BurstShotsRemaining <= 0)
		{
			FinishedBurst();
			if (bAllowContinuousBurstFire)
			{
				BurstShotsRemaining = BurstCount;
				// We will force a cooldown for the full burst interval, regardless of the time already consumed in the previous burst, for simplicity.
				ForceCooldown(BurstInterval);
			}
			else
			{
				if (GetMovementComponent())
				{
					GetMovementComponent()->SetIsBusy(false);
				}
				IsPrimaryUsing = false;
			}
		}
	}

}

FVector AInstantWeapon::GetLineTraceDirection()
{
	FVector Direction = Super::GetLineTraceDirection();

	float SpreadToUse = SpreadAt100m;
	if (GetMovementComponent())
	{
		if (GetMovementComponent()->IsAiming())
		{
			SpreadToUse = SpreadAt100mWhenAiming;
		}
		if (GetMovementComponent()->IsCrouching())
		{
			SpreadToUse *= SpreadCrouchModifier;
		}
	}

	if (SpreadToUse > 0)
	{
		auto Spread = FMath::RandPointInCircle(SpreadToUse);
		Direction = Direction.Rotation().RotateVector(FVector(10000, Spread.X, Spread.Y));
	}

	return Direction;
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
	
	bool bFlag = GetGameInstance()->IsDedicatedServerInstance();
	FString serverString = bFlag ? "SERVER" : "CLIENT";
	UE_LOG(LogGDK, Warning, TEXT("AInstantWeapon::DealDamage, %s"), *serverString);

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		HitInfo.HitActor->TakeDamage(ShotBaseDamage, DmgEvent, Pawn->GetController(), this);

		REAL_BLAST_MESH_ACTOR* blastActor = Cast<REAL_BLAST_MESH_ACTOR>(HitInfo.HitActor);
		UE_LOG(LogGDK, Warning, TEXT("AInstantWeapon::DealDamage 1111111111111111, %s"), *serverString);

		if (blastActor)
		{
			UE_LOG(LogGDK, Warning, TEXT("AInstantWeapon::DealDamage 22222222222222222222, %s"), *serverString);
			REAL_BLAST_MESH_COMPONENT* blastComp = Cast<REAL_BLAST_MESH_COMPONENT>(blastActor->GetBlastMeshComponent());
			if (blastComp)
			{
				UE_LOG(LogGDK, Warning, TEXT("AInstantWeapon::DealDamage 3333333333333333333333333, %s"), *serverString);
				blastComp->ApplyRadialDamage(HitInfo.Location, 80, 200, 500, 1000);
			}
		}
	}
}

bool AInstantWeapon::ServerDidHit_Validate(const FInstantHitInfo& HitInfo)
{
	return true;
}

void AInstantWeapon::ServerDidHit_Implementation(const FInstantHitInfo& HitInfo)
{
	bool bFlag = GetGameInstance()->IsDedicatedServerInstance();
	FString serverString = bFlag ? "SERVER" : "CLIENT";
	UE_LOG(LogGDK, Warning, TEXT("AInstantWeapon::ServerDidHit_Implementation, %s"), *serverString);
	bool bDoNotifyHit = false;

	if (HitInfo.HitActor == nullptr)
	{
		UE_LOG(LogGDK, Warning, TEXT("AInstantWeapon::ServerDidHit_Implementation 1111111, %s"), *serverString);
		bDoNotifyHit = true;
	}
	else
	{
		if (ValidateHit(HitInfo))
		{
			UE_LOG(LogGDK, Warning, TEXT("AInstantWeapon::ServerDidHit_Implementation 222222222222222, %s"), *serverString);
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
	NotifyClientsOfHit(HitInfo, false);
}

void AInstantWeapon::MulticastNotifyHit_Implementation(FInstantHitInfo HitInfo, bool bImpact)
{
	// Make sure we're a client, and we're not the client that owns this gun (they will have already played the effect locally).
	APawn* Pawn = Cast<APawn>(GetOwner());

	if (GetNetMode() != NM_DedicatedServer &&
		(Pawn == nullptr || !Pawn->IsLocallyControlled()))
	{
		SpawnFX(HitInfo, bImpact);
	}
}

void AInstantWeapon::SetIsActive(bool bNewActive)
{
	Super::SetIsActive(bNewActive);

	ConsumeBufferedShot();
}
