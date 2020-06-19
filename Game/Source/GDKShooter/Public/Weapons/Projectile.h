// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Weapons/Weapon.h"
#include "Projectile.generated.h"

UCLASS(Abstract, Blueprintable)
class GDKSHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetPlayer(AWeapon* Weapon);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MetaData)
	FGDKMetaData MetaData;

	UFUNCTION(BlueprintImplementableEvent)
	void OnMetaDataUpdated();

	UPROPERTY(Handover)
	AWeapon* InstigatingWeapon;

protected:
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;

	UFUNCTION()
	virtual void OnStop(const FHitResult& ImpactResult);
	UFUNCTION()
	virtual void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION(BlueprintNativeEvent)
	void OverlapPawn(APawn* Pawn);

	// If doesn't explode on stop, will explode on timer
	// Later we can add proximity mines etc.
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	bool ExplodeOnStop = true;

	// Explode after X bounces if > 0
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	int MaximumBounces = -1;

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float LifeTillExplode = 5;

	// Visible mesh.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Mesh;

	int BouncesSoFar = 0;

	float BeginTime;

	UFUNCTION()
	void OnRep_MetaData();

	UFUNCTION(BlueprintCallable)
	virtual void Explode();

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Exploded)
	bool bExploded;

	UFUNCTION()
	void OnRep_Exploded();

	UFUNCTION(BlueprintNativeEvent)
	void ExplosionVisuals();

	UPROPERTY(Handover)
	AController* InstigatingController;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Projectile)
	float ExplosionDamage = 50;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Projectile)
	float ExplosionMinimumDamage = 10;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Projectile)
	float ExplosionRadius = 500;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Projectile)
	float ExplosionInnerRadius = 100;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Projectile)
	float ExplosionFalloff = 1;

	// Type of damage to send to hit actors.
	UPROPERTY(EditAnywhere, Category = Projectile)
	TSubclassOf<UDamageType> DamageTypeClass;
	
	UPROPERTY(Category = Projectile, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* MovementComp;

	UPROPERTY(Category = Projectile, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USphereComponent* CollisionComp;
};
