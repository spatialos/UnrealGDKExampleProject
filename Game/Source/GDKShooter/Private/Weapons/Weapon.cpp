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
	return Shooting->GetLineTraceDirection();
}

void AWeapon::AnnounceShot(bool bHit)
{
	if (Shooting)
	{
		Shooting->FireShot(this);
	}
}

void AWeapon::DoFire_Implementation()
{

}

FInstantHitInfo AWeapon::DoLineTrace()
{
	FInstantHitInfo OutHitInfo;

	if (Wielder == nullptr)
	{
		UE_LOG(LogGDK, Verbose, TEXT("Weapon %s does not have an owning character"), *this->GetName());
		return OutHitInfo;
	}

	FCollisionQueryParams TraceParams;
	TraceParams.bTraceComplex = true;
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(this);
	TraceParams.AddIgnoredActor(Wielder->GetOwner());

	if (bDrawDebugLineTrace)
	{
		TraceParams.TraceTag = kTraceTag;
	}

	FHitResult HitResult(ForceInit);
	FVector TraceStart = Shooting->GetLineTraceStart();
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

	Wielder->SetBusy(true);
}

void AWeapon::StopPrimaryUse_Implementation()
{
	Super::StopPrimaryUse_Implementation();

	if (!HasBufferedShot())
	{
		Wielder->SetBusy(false);
	}
}

void AWeapon::StartSecondaryUse_Implementation()
{
	Super::StartSecondaryUse_Implementation();

	if (Movement)
	{
		Movement->SetAiming(true);
		Movement->SetAimingRotationModifier(AimingRotationSpeed);
	}
}

void AWeapon::StopSecondaryUse_Implementation()
{
	Super::StopSecondaryUse_Implementation();

	if (Movement)
	{
		Movement->SetAiming(false);
	}
}

void AWeapon::OnRep_Wielder()
{
	Super::OnRep_Wielder();

	UE_LOG(LogGDK, Error, TEXT("OnRep_Wielder for %s"), *this->GetName());
	if (Wielder)
	{
		Movement = Cast<UGDKMovementComponent>(Wielder->GetOwner()->GetComponentByClass(UGDKMovementComponent::StaticClass()));
		Shooting = Cast<UShootingComponent>(Wielder->GetOwner()->GetComponentByClass(UShootingComponent::StaticClass()));
		UE_LOG(LogGDK, Error, TEXT("Got Shooting? %d"), (Shooting ? 1 : 0));
	}
	else
	{
		Movement = nullptr;
		Shooting = nullptr;
		UE_LOG(LogGDK, Error, TEXT("Shooting = nullptr"));
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

	if (GetNetMode() != NM_Client)
	{
		return;
	}

	if ((IsPrimaryUsing || HasBufferedShot()) && ReadyToFire())
	{
		ConsumeBufferedShot();
		DoFire();

		if (!IsPrimaryUsing)
		{
			Wielder->SetBusy(false);
		}
	}

	if (HasBufferedShot() && !BufferedShotStillValid())
	{
		ConsumeBufferedShot();
		if (!IsPrimaryUsing)
		{
			Wielder->SetBusy(false);
		}
	}
}

void AWeapon::SetIsActive(bool bNewIsActive)
{
	Super::SetIsActive(bNewIsActive);

	ConsumeBufferedShot();
}
