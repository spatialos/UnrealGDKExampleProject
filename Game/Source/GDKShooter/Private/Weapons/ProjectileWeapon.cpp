// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ProjectileWeapon.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Weapons/Projectile.h"
#include "Characters/Core/GDKEquippedCharacter.h"
#include "GDKLogging.h"
#include "Components/SkeletalMeshComponent.h"

AProjectileWeapon::AProjectileWeapon()
{
	ShotCooldown = 1;
}

void AProjectileWeapon::DoFire()
{
	float Now = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	NextShotTime = Now + ShotCooldown;

	FVector Direction = Shooting->GetLineTraceDirection();

	FVector Barrel = Mesh->GetSocketLocation(BarrelSocket);


	FInstantHitInfo HitInfo;
	if (DoLineTrace(HitInfo))
	{
		Direction = HitInfo.Location - Barrel;
		Direction.Normalize();
	}
	else
	{
		Direction = Shooting->GetLineTraceDirection();
		Direction.Normalize();
	}

	AnnounceShot(false);
	OnShot();
	FireProjectile(Barrel, Direction);
	Wielder->SetBusy(false);
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
	Wielder->SetBusy(false);
}
