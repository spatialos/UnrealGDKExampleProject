// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"

#if UE_VERSION_OLDER_THAN(4, 27, 0)
#include "Camera/CameraShake.h"
#else
#include "Camera/CameraShakeBase.h"
#endif
#include "Characters/Components/GDKMovementComponent.h"
#include "Characters/Components/ShootingComponent.h"
#include "GameFramework/Actor.h"
#include "Holdable.h"
#include "Materials/Material.h"
#include "TimerManager.h"
#include "Weapon.generated.h"

UCLASS(Abstract)
class GDKSHOOTER_API AWeapon : public AHoldable
{
	GENERATED_BODY()

public:
	AWeapon();

	virtual void Tick(float DeltaTime) override;

	virtual void StartSecondaryUse_Implementation() override;
	virtual void StopSecondaryUse_Implementation() override;

	virtual void SetIsActive(bool bNewActive);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AimingFoV = 55;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AimingRotationSpeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInstance* Reticle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D ReticleSize = FVector2D(16, 16);

	virtual void ForceCooldown(float Cooldown) override;

	//Recoil applied as input to the CharacterController rotations
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RecoilYawMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RecoilYawMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RecoilRightBias = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RecoilPitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RecoilPitchMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RecoilUpBias = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RecoilCrouchModifier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RecoilAimModifier = 0.5f;

	//Rate at which recoil is recovered from
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RecoilRecoveryRate = 1;

	//Recoil applied as a CameraShake
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UCameraShakeBase> RecoilShake;

	virtual void StartPrimaryUse_Implementation() override;
	virtual void StopPrimaryUse_Implementation() override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintNativeEvent)
	void DoFire();

	virtual bool ReadyToFire();

	virtual void AnnounceShot(bool bHit);

	virtual FVector GetLineTraceDirection();

	UFUNCTION(BlueprintPure)
	UGDKMovementComponent* GetMovementComponent();
	UFUNCTION(BlueprintPure)
	UShootingComponent* GetShootingComponent();

	UFUNCTION(BlueprintPure)
	FInstantHitInfo DoLineTrace();

	// Time that we are next able to shoot
	float NextShotTime;
	// Buffered shots are for when e.g. people double click just slightly faster than the RoF
	// or a single shot that needs to wait for the sprint cooldown
	float BufferedShotUntil;
	float BufferShotThreshold;
	bool bHasBufferedShot;
	bool HasBufferedShot();
	bool BufferedShotStillValid();
	virtual void ConsumeBufferedShot();

private:
	UPROPERTY()
	AActor* CachedOwner;

	UPROPERTY()
	UGDKMovementComponent* CachedMovementComponent;

	UPROPERTY()
	UShootingComponent* CachedShootingComponent;

	UFUNCTION()
	void RefreshComponentCache();
};
