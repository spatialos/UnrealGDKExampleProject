// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKShooterCharacter.h"

#include "Weapons/InstantWeapon.h"
#include "Components/GDKMovementComponent.h"
#include "Controllers/GDKPlayerController.h"
#include "Engine/World.h"
#include "GDKLogging.h"
#include "UnrealNetwork.h"

AGDKShooterCharacter::AGDKShooterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EquippedWeapon = nullptr;
}

void AGDKShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Only spawn a new starter weapon if we're authoritative and don't already have one.
	if (HasAuthority())
	{
		SpawnStarterWeapons();
	}
}


void AGDKShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGDKShooterCharacter, EquippedWeapon);

	DOREPLIFETIME(AGDKShooterCharacter, bIsAiming);
}

// Called to bind functionality to input
void AGDKShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AGDKShooterCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AGDKShooterCharacter::StopFire);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AGDKShooterCharacter::StartAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AGDKShooterCharacter::StopAim);
	PlayerInputComponent->BindAction< FWeaponSelection>("1", IE_Pressed, this, &AGDKShooterCharacter::EquipWeaponLocally, 0);
	PlayerInputComponent->BindAction< FWeaponSelection>("2", IE_Pressed, this, &AGDKShooterCharacter::EquipWeaponLocally, 1);
	PlayerInputComponent->BindAction< FWeaponSelection>("3", IE_Pressed, this, &AGDKShooterCharacter::EquipWeaponLocally, 2);
	PlayerInputComponent->BindAction< FWeaponSelection>("4", IE_Pressed, this, &AGDKShooterCharacter::EquipWeaponLocally, 3);
	PlayerInputComponent->BindAction< FWeaponSelection>("5", IE_Pressed, this, &AGDKShooterCharacter::EquipWeaponLocally, 4);
	PlayerInputComponent->BindAction< FWeaponSelection>("6", IE_Pressed, this, &AGDKShooterCharacter::EquipWeaponLocally, 5);
	PlayerInputComponent->BindAction< FWeaponSelection>("7", IE_Pressed, this, &AGDKShooterCharacter::EquipWeaponLocally, 6);
	PlayerInputComponent->BindAction< FWeaponSelection>("8", IE_Pressed, this, &AGDKShooterCharacter::EquipWeaponLocally, 7);
	PlayerInputComponent->BindAction< FWeaponSelection>("9", IE_Pressed, this, &AGDKShooterCharacter::EquipWeaponLocally, 8);
	PlayerInputComponent->BindAction< FWeaponSelection>("0", IE_Pressed, this, &AGDKShooterCharacter::EquipWeaponLocally, 9);
	PlayerInputComponent->BindAction< FWeaponSelection>("-", IE_Pressed, this, &AGDKShooterCharacter::EquipWeaponLocally, 10);
	PlayerInputComponent->BindAction< FWeaponSelection>("+", IE_Pressed, this, &AGDKShooterCharacter::EquipWeaponLocally, 11);
}

void AGDKShooterCharacter::AttachWeapon(AWeapon* Weapon) const
{
	Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, kRightGunSocketName);
	Weapon->EnableShadows(true);
	Weapon->SetFirstPerson(false);
}

void AGDKShooterCharacter::AddAimingListener(FAimingStateChanged Listener)
{
	AimingCallback = Listener;
}

void AGDKShooterCharacter::AddWeaponListener(FWeaponChanged Listener)
{
	WeaponCallback = Listener;
}

bool AGDKShooterCharacter::IgnoreActionInput() const
{
	check(GetNetMode() != NM_DedicatedServer);

	if (AGDKPlayerController* PC = Cast<AGDKPlayerController>(GetController()))
	{
		return PC->IgnoreActionInput();
	}
	return false;
}

void AGDKShooterCharacter::StartFire()
{
	check(GetNetMode() != NM_DedicatedServer);
	bFireHeld = true;
	if (IgnoreActionInput())
	{
		return;
	}
	
	if (EquippedWeapon != nullptr)
	{
		// Don't allow sprinting and shooting at the same time.
		StopSprinting();

		EquippedWeapon->StartFire();
	}
}

void AGDKShooterCharacter::StopFire()
{
	check(GetNetMode() != NM_DedicatedServer);
	bFireHeld = false;
	
	if (EquippedWeapon != nullptr)
	{
		EquippedWeapon->StopFire();
	}
}

bool AGDKShooterCharacter::CanFire()
{
	return EquippedWeapon && !HasSprintedRecently();
}

void AGDKShooterCharacter::StartAim()
{
	bIsAiming = true;
	SetAiming(bIsAiming);
	if (UGDKMovementComponent* MovementComponent = Cast<UGDKMovementComponent>(GetCharacterMovement()))
	{
		MovementComponent->SetAiming(true);
	}
	OnStartedAiming();
	AimingCallback.ExecuteIfBound(true, EquippedWeapon->AimingRotationSpeed);
}

void AGDKShooterCharacter::StopAim()
{
	bIsAiming = false;
	SetAiming(bIsAiming);
	if (UGDKMovementComponent* MovementComponent = Cast<UGDKMovementComponent>(GetCharacterMovement()))
	{
		MovementComponent->SetAiming(false);
	}
	OnStoppedAiming();
	AimingCallback.ExecuteIfBound(false, 1);
}

bool AGDKShooterCharacter::SetAiming_Validate(bool NewValue)
{
	return true;
}

void AGDKShooterCharacter::SetAiming_Implementation(bool NewValue)
{
	bIsAiming = NewValue;
}

bool AGDKShooterCharacter::IsAiming()
{
	return bIsAiming;
}

void AGDKShooterCharacter::SpawnStarterWeapons()
{
	if (!HasAuthority())
	{
		return;
	}

	for (int i=0; i<StarterWeaponTemplates.Num(); i++)
	{
		SpawnStarterWeapon(StarterWeaponTemplates[i], i == 0);
	}
}

void AGDKShooterCharacter::SpawnStarterWeapon(TSubclassOf<AWeapon> Weapon, bool bIsActive)
{
	if (!HasAuthority())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	AWeapon* StartWeapon = GetWorld()->SpawnActor<AWeapon>(Weapon, GetActorTransform(), SpawnParams);
	StartWeapon->SetOwningCharacter(this);
	StartWeapon->SetIsActive(bIsActive);
	StartWeapon->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	StartWeapon->SetMetaData(MetaData);

	AvailableWeapons.Add(StartWeapon);

	if (bIsActive)
	{
		EquippedWeapon = StartWeapon;
	}
}

void AGDKShooterCharacter::EquipWeaponLocally(int32 WeaponId)
{
	StopAim();
	EquipWeapon(WeaponId);
}

bool AGDKShooterCharacter::EquipWeapon_Validate(int32 WeaponId)
{
	return true;
}

void AGDKShooterCharacter::OnRep_EquippedWeapon()
{
	if (CachedEquippedWeapon)
	{
		CachedEquippedWeapon->RemoveShotListener();
	}
	if (EquippedWeapon)
	{
		FShotDelegate ShotCallback;
		ShotCallback.BindUObject(this, &AGDKShooterCharacter::OnShot);
		EquippedWeapon->AddShotListener(ShotCallback);
	}
	CachedEquippedWeapon = EquippedWeapon;
	WeaponCallback.ExecuteIfBound(EquippedWeapon);
	OnEquippedWeaponChanged();
}

void AGDKShooterCharacter::AddShotListener(FWeaponShotDelegate Listener)
{
	ShotCallback = Listener;
}

void AGDKShooterCharacter::OnShot(bool Hit)
{
	ShotCallback.ExecuteIfBound(EquippedWeapon, Hit);
}

void AGDKShooterCharacter::EquipWeapon_Implementation(int32 WeaponId)
{
	if (AvailableWeapons.Num() <= WeaponId)
	{
		return;
	}

	if (EquippedWeapon == AvailableWeapons[WeaponId])
	{
		return;
	}

	EquippedWeapon->SetIsActive(false);
	EquippedWeapon = AvailableWeapons[WeaponId];
	EquippedWeapon->SetIsActive(true);
}


void AGDKShooterCharacter::SetMetaData(FGDKMetaData MetaData)
{
	Super::SetMetaData(MetaData);

	for (AWeapon* Weapon : AvailableWeapons)
	{
		if (Weapon)
		{
			Weapon->SetMetaData(MetaData);
		}
	}
}


void AGDKShooterCharacter::Die(const AGDKCharacter* Killer)
{
	Super::Die(Killer);
	
	if (EquippedWeapon != nullptr)
	{
		EquippedWeapon->StopFire();
	}

	if (GetNetMode() == NM_DedicatedServer && HasAuthority())
	{
		if (EquippedWeapon != nullptr && !EquippedWeapon->IsPendingKill())
		{
			GetWorld()->DestroyActor(EquippedWeapon);
		}
	}
}

void AGDKShooterCharacter::Destroyed()
{
	Super::Destroyed();

	if (GetNetMode() == NM_DedicatedServer && HasAuthority())
	{
		for (AWeapon* Weapon : AvailableWeapons)
		{
			if (Weapon != nullptr && !Weapon->IsPendingKill())
			{
				GetWorld()->DestroyActor(Weapon);
			}
		}
	}
}
