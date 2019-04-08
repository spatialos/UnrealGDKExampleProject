// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "InstantWeapon.generated.h"

/**
 * AInstantWeapon implements hitscan shooting for a single-shot, burst-fire, or full-auto weapon.
 * Hit detection is entirely client-side, with loose server validation.
 * Shot timing and rate-limiting is entirely client-side, with no server validation.
 */
UCLASS(Abstract, Blueprintable, SpatialType)
class GDKSHOOTER_API AInstantWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	AInstantWeapon();

	virtual void StartFire() override;

	virtual void StopFire() override;

	// RPC for telling the server that we fired and hit something.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDidHit(const FInstantHitInfo& HitInfo);
	bool ServerDidHit_Validate(const FInstantHitInfo& HitInfo);
	void ServerDidHit_Implementation(const FInstantHitInfo& HitInfo);

	// RPC for telling the server that we fired and did not hit anything.
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerDidMiss(const FInstantHitInfo& HitInfo);
	bool ServerDidMiss_Validate(const FInstantHitInfo& HitInfo);
	void ServerDidMiss_Implementation(const FInstantHitInfo& HitInfo);

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapons")
	void OnRenderShot(const FVector Location, bool bImpact);

	UFUNCTION(BlueprintImplementableEvent)
		void FinishedBurst();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void Tick(float DeltaTime) override;

	// [client] Runs a line trace and triggers the server RPC for hits.
	virtual void DoFire();

	virtual FVector GetLineTraceDirection() override;

private:

	// [server] Notifies clients of a hit.
	void NotifyClientsOfHit(const FInstantHitInfo& HitInfo, bool bImpact);

	// [client] Spawns the hit FX in the world.
	void SpawnFX(const FInstantHitInfo& HitInfo, bool bImpact);

	// [server] Validates the hit. Returns true if it's valid, false otherwise.
	bool ValidateHit(const FInstantHitInfo& HitInfo);

	// [server] Actually deals damage to the actor we hit.
	void DealDamage(const FInstantHitInfo& HitInfo);

	// [client] Clears the NextShotTimer if it's running.
	void ClearTimerIfRunning();

	// [client] Actually stops the weapon firing.
	void StopFiring();

	void ActuallyStartFiring();

	bool ReadyToStartFiring();

	bool ReadyToFire();

	// Notifies all clients that a shot hit something. Used for visualizing shots on the client.
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastNotifyHit(FInstantHitInfo HitInfo, bool bImpact);

	// Returns true if the weapon is a burst-fire weapon.
	FORCEINLINE bool IsBurstFire()
	{
		return BurstCount >= 1;
	}

	// Returns true if the weapon is fully-automatic.
	FORCEINLINE bool IsFullyAutomatic()
	{
		return BurstCount < 1;
	}

	// Minimum time between bursts (or shots, if in single-shot or automatic mode), in seconds.
	// 0 = as fast as you can pull the trigger
	UPROPERTY(EditAnywhere, Category = "Weapons")
		float BurstInterval;

	// Interval between individual shots, in seconds.
	UPROPERTY(EditAnywhere, Category = "Weapons")
		float ShotInterval;

	// Number of shots in a single burst.
	// 0  = full-auto
	// 1  = single-shot
	// >1 = burst fire
	UPROPERTY(EditAnywhere, Category = "Weapons")
	int32 BurstCount;

	// Amount of time to buffer a shot for if fired just before off cooldown
	UPROPERTY(EditAnywhere, Category = "Weapons")
		float ShotInputLeniancy;

	// Time for which we have a shot buffered
	float ShotBufferedUntil;

	// If we have a shot buffered
	bool  ShotBuffered;

	// Time (in seconds since start of level) since the last burst fire. Used for limiting fire rate.
	float LastBurstTime;

	// Time (in seconds since start of level) since the last shot fired. Used for limiting fire rate.
	float LastShotTime;

	// Number of shots remaining in the current burst.
	int32 BurstShotsRemaining;
	
	// Base damage done to others by a single shot.
	UPROPERTY(EditAnywhere, Category = "Weapons")
	float ShotBaseDamage;

	// Tolerance, in world units, to add to the bounding box of an actor when validating hits.
	UPROPERTY(EditAnywhere, Category = "Weapons")
	float HitValidationTolerance;

	// Type of damage to send to hit actors.
	UPROPERTY(EditAnywhere, Category = "Weapons")
	TSubclassOf<UDamageType> DamageTypeClass;

	// Tolerance, in seconds, after which we will no longer visualize a shot notification.
	UPROPERTY(EditAnywhere, Category = "Weapons")
		FTimespan ShotVisualizationDelayTolerance;

	UPROPERTY(EditAnywhere, Category = "Weapons")
		float SpreadAt100m = 0;

	UPROPERTY(EditAnywhere, Category = "Weapons")
		float SpreadAt100mWhenAiming = 0;
};
