// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Projectile.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "GDKLogging.h"
#include "UnrealNetwork.h"

AProjectile::AProjectile(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	bReplicates = true;
	bReplicateMovement = true;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);

	CollisionComp = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(0.5f);
	CollisionComp->AlwaysLoadOnClient = true;
	CollisionComp->AlwaysLoadOnServer = true;
	CollisionComp->bTraceComplexOnMove = true;
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = CollisionComp;

	MovementComp = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileComp"));
	MovementComp->UpdatedComponent = CollisionComp;
	MovementComp->InitialSpeed = 2000.0f;
	MovementComp->MaxSpeed = 2000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.5f;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisibleMesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	CollisionComp->MoveIgnoreActors.Add(Instigator);
	MovementComp->OnProjectileStop.AddDynamic(this, &AProjectile::OnStop);
	MovementComp->OnProjectileBounce.AddDynamic(this, &AProjectile::OnBounce);
	BeginTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
}

void AProjectile::SetPlayer(AGDKCharacter* Character, AWeapon* Weapon)
{
	CollisionComp->MoveIgnoreActors.Add(Character);
	InstigatingController = Character->GetController();
	InstigatingWeapon = Weapon;
}

void AProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectile, bExploded);
	DOREPLIFETIME(AProjectile, MetaData);
}

void AProjectile::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	if (MovementComp)
	{
		MovementComp->Velocity = NewVelocity;
	}
}

void AProjectile::OnRep_MetaData()
{
	OnMetaDataUpdated();
}

void AProjectile::Tick(float DeltaTime)
{
	if (GetNetMode() == NM_Client)
	{
		return;
	}
	
	float Now = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	if (!bExploded && Now > BeginTime + LifeTillExplode)
	{
		Explode();
	}

}

void AProjectile::OnStop(const FHitResult& ImpactResult)
{
	if (GetNetMode() == NM_Client)
	{
		return;
	}

	if (ExplodeOnStop && !bExploded)
	{
		Explode();
	}
}

void AProjectile::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (GetNetMode() == NM_Client)
	{
		return;
	}

	BouncesSoFar++;
	if (MaximumBounces >= 0 && BouncesSoFar > MaximumBounces && !bExploded)
	{
		Explode();
	}
}


void AProjectile::OnRep_Exploded()
{
	ExplosionVisuals();
}

void AProjectile::ExplosionVisuals_Implementation()
{
	Mesh->SetVisibility(false, true);
}

void AProjectile::Explode()
{
	if (GetNetMode() == NM_Client)
	{
		return;
	}

	bCanBeDamaged = false;
	bExploded = true;
	MovementComp->StopMovementImmediately();
	SetLifeSpan(2.0f);
	if (ExplosionDamage > 0 && ExplosionRadius > 0)
	{
		UGameplayStatics::ApplyRadialDamageWithFalloff(this, ExplosionDamage, ExplosionMinimumDamage, this->GetActorLocation(), ExplosionInnerRadius, ExplosionRadius, ExplosionFalloff, DamageTypeClass, TArray<AActor*>{this}, InstigatingWeapon, InstigatingController);
	}
}
