// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ProjectileWeapon.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Weapons/Projectile.h"
#include "Characters/Core/GDKShooterCharacter.h"
#include "GDKLogging.h"
#include "Components/SkeletalMeshComponent.h"

AProjectileWeapon::AProjectileWeapon()
{
	ShotCooldown = 1;
	LastShotTime = 0;
}

void AProjectileWeapon::StartFire()
{
	check(GetNetMode() == NM_Client);

	float Now = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	if (GetWeaponState() == EWeaponState::Idle && Now > LastShotTime + ShotCooldown)
	{
		LastShotTime = Now;

		FVector Direction = GetOwningCharacter()->GetLineTraceDirection();

		FVector Barrel = Mesh->GetSocketLocation(BarrelSocket);


		FInstantHitInfo HitInfo;
		if (DoLineTrace(HitInfo))
		{
			Direction = HitInfo.Location - Barrel;
			Direction.Normalize();
		}
		else
		{
			Direction = GetOwningCharacter()->GetLineTraceDirection();
			Direction.Normalize();
		}

		AnnounceShot(false);
		OnShot();
		FireProjectile(Barrel, Direction);
	}
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
		Projectile->SetPlayer(GetOwningCharacter(), this);
		Projectile->MetaData = MetaData;
		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTransformMatrix);
	}
}
