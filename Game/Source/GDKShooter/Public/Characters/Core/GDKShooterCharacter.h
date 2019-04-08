// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GDKMobileCharacter.h"
#include "Weapons/Weapon.h"
#include "GDKShooterCharacter.generated.h"


DECLARE_DELEGATE_TwoParams(FAimingStateChanged, bool, float);
DECLARE_DELEGATE_OneParam(FWeaponSelection, int32);
DECLARE_DELEGATE_OneParam(FWeaponChanged, AWeapon*);
DECLARE_DELEGATE_TwoParams(FWeaponShotDelegate, AWeapon*, bool);
UCLASS()
class GDKSHOOTER_API AGDKShooterCharacter : public AGDKMobileCharacter
{
	GENERATED_BODY()

public:
	AGDKShooterCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// [server + client] Returns true if the character is able to shoot at the given moment.
	bool CanFire();

	// Returns a position from which to start a line trace.
	// Use this so your line trace doesn't collide with the player character.
	virtual FVector GetLineTraceStart() const PURE_VIRTUAL(AGDKShooterCharacter::GetLineTraceStart, return FVector::ZeroVector;);

	// Returns the direction in which to perform a line trace so it lines up with the center of the crosshair.
	virtual FVector GetLineTraceDirection() const PURE_VIRTUAL(AGDKShooterCharacter::GetLineTraceDirection, return FVector::ZeroVector;);

	virtual void AttachWeapon(AWeapon* weapon) const;

	// [server + client] Returns true if the character is currently sprinting.
	UFUNCTION(BlueprintPure, Category = "Aiming")
		bool IsAiming();

	// Called on clients when the player health changes
	UFUNCTION(BlueprintImplementableEvent, Category = "Aiming")
		void OnStartedAiming();

	// Called on clients when the player health changes
	UFUNCTION(BlueprintImplementableEvent, Category = "Aiming")
		void OnStoppedAiming();

	UFUNCTION(Server, Reliable, WithValidation)
		void EquipWeapon(int32 weapon);
	UFUNCTION(BlueprintCallable)
		void EquipWeaponLocally(int32 weapon);

	void StartFire();

	void StopFire();

	void AddAimingListener(FAimingStateChanged Listener);
	void AddWeaponListener(FWeaponChanged Listener);
	void AddShotListener(FWeaponShotDelegate Listener);

	virtual void SetMetaData(FGDKMetaData MetaData) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicatedusing=OnRep_EquippedWeapon)
		class AWeapon* EquippedWeapon;

protected:

	UPROPERTY(EditDefaultsOnly)
		FName kRightGunSocketName = FName(TEXT("Gun_Transform"));

	UPROPERTY(Transient)
		class AWeapon* CachedEquippedWeapon;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	// [client] If true, the character should ignore all action inputs.
	bool IgnoreActionInput() const;

	void StartAim();

	void StopAim();

	// Debug RPC to tell the server to aim or not
	UFUNCTION(Server, Reliable, WithValidation)
		void SetAiming(bool NewValue);

	virtual void Die(const class AGDKCharacter* Killer) override;

	UFUNCTION()
		virtual void OnRep_EquippedWeapon();

	UFUNCTION(BlueprintImplementableEvent)
		void OnEquippedWeaponChanged();

	virtual void OnShot(bool Hit);

private:

	UPROPERTY(VisibleAnywhere, Replicated)
		TArray<AWeapon*> AvailableWeapons;

	UPROPERTY(VisibleAnywhere, Replicated)
		bool bIsAiming;

	bool bFireHeld;

	FAimingStateChanged AimingCallback;
	FWeaponChanged WeaponCallback;
	FWeaponShotDelegate ShotCallback;

	// Weapons available to the player, and whether they are available on spawn
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
		TArray<TSubclassOf<AWeapon>> StarterWeaponTemplates;

	// [server] Spawns the starter weapons and attaches them to the character.
	void SpawnStarterWeapons();

	// [server] Spawns a starter weapon and attaches it to the character.
	void SpawnStarterWeapon(TSubclassOf<AWeapon> Weapon, bool bIsActive);
};
