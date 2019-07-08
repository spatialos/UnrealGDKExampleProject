// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ProjectileWeapon.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Weapons/Projectile.h"
#include "GDKLogging.h"
#include "Components/SkeletalMeshComponent.h"

AProjectileWeapon::AProjectileWeapon()
{
	ShotCooldown = 1;
}

void AProjectileWeapon::DoFire_Implementation()
{
	if (!GetShootingComponent())
	{
		UE_LOG(LogGDK, Verbose, TEXT("Weapon %s does not have a shooting component"), *this->GetName());
		return;
	}

	float Now = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	NextShotTime = Now + ShotCooldown;

	FVector Direction = GetShootingComponent()->GetLineTraceDirection();

	FVector Barrel = Mesh->GetSocketLocation(BarrelSocket);


	FInstantHitInfo HitInfo = DoLineTrace();
	if (HitInfo.bDidHit)
	{
		Direction = HitInfo.Location - Barrel;
		Direction.Normalize();
	}
	else
	{
		Direction = GetShootingComponent()->GetLineTraceDirection();
		Direction.Normalize();
	}

	AnnounceShot(false);
	OnShot();
	FireProjectile(Barrel, Direction);
	if (GetMovementComponent())
	{
		GetMovementComponent()->SetIsBusy(false);
	}
	IsPrimaryUsing = false;
}

bool AProjectileWeapon::FireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal Direction)
{
	return true;
}

void AProjectileWeapon::FireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal Direction)
{
	FTransform SpawnTransformMatrix(Direction.Rotation(), Origin);

	AProjectile* Projectile = Cast<AProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjectileClass, SpawnTransformMatrix));
	if (Projectile)
	{
		Projectile->SetPlayer(this);
		Projectile->MetaData = MetaData;
		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTransformMatrix);
	}
}

void AProjectileWeapon::ConsumeBufferedShot()
{
	Super::ConsumeBufferedShot();
	IsPrimaryUsing = false;
	if (GetMovementComponent())
	{
		GetMovementComponent()->SetIsBusy(false);
	}
}
