// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SpatialNetDriver.h"
#include "UnrealNetwork.h"
#include "GDKLogging.h"
#include "Game/GDKPlayerState.h"
#include "Controllers/GDKPlayerController.h"
#include "Weapons/InstantWeapon.h"

// Sets default values
AGDKCharacter::AGDKCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	MaxHealth = 100;
	CurrentHealth = MaxHealth;
	MaxArmour = 100;
	CurrentArmour = 0;
	bIsRagdoll = false;
}

// Called when the game starts or when spawned
void AGDKCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CurrentHealth = MaxHealth;
		CurrentArmour = 0;
	}
	OnMetaDataUpdated();

}

void AGDKCharacter::Destroyed()
{
	Super::Destroyed();

	if (GetWorldTimerManager().IsTimerActive(RegenerationHandle))
	{
		GetWorldTimerManager().ClearTimer(RegenerationHandle);
	}
}

void AGDKCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGDKCharacter, bIsRagdoll);

	DOREPLIFETIME(AGDKCharacter, CurrentHealth);

	DOREPLIFETIME(AGDKCharacter, CurrentArmour);

	DOREPLIFETIME(AGDKCharacter, MetaData);
}

void AGDKCharacter::Die(const AGDKCharacter* Killer)
{
	TearOff();

	if (GetNetMode() == NM_DedicatedServer && HasAuthority())
	{
		AGDKPlayerController* PC = Cast<AGDKPlayerController>(GetController());
		if (PC)
		{
			PC->KillCharacter(Killer);
		}

		bIsRagdoll = true;
		OnRep_IsRagdoll();
	}
}

void AGDKCharacter::StartRagdoll()
{
	// Disable capsule collision and disable movement.
	UCapsuleComponent* CapsuleComponent = GetCapsuleComponent();
	if (CapsuleComponent == nullptr)
	{
		UE_LOG(LogGDK, Error, TEXT("Invalid capsule component on character %s"), *this->GetName());
		return;
	}

	CapsuleComponent->SetSimulatePhysics(false);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// Enable mesh collision and physics.
	USkeletalMeshComponent* MeshComponent = GetMesh();
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	MeshComponent->SetCollisionProfileName(FName(TEXT("Ragdoll")));
	MeshComponent->SetSimulatePhysics(true);

	// Gather list of child components of the capsule.
	TArray<USceneComponent*> ComponentsToMove;
	int NumChildren = CapsuleComponent->GetNumChildrenComponents();
	for (int i = 0; i < NumChildren; ++i)
	{
		USceneComponent* Component = CapsuleComponent->GetChildComponent(i);
		if (Component != nullptr && Component != MeshComponent)
		{
			ComponentsToMove.Add(Component);
		}
	}

	SetRootComponent(MeshComponent);

	// Move the capsule's former child components over to the mesh.
	for (USceneComponent* Component : ComponentsToMove)
	{
		Component->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	// Fix up the camera to a "death view".
	if (GetNetMode() == NM_Client)
	{
		ShowRespawnScreen();
	}
}

void AGDKCharacter::OnRep_CurrentHealth()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		AGDKPlayerController* PC = Cast<AGDKPlayerController>(GetController());
		if (PC)
		{
			PC->UpdateHealthUI(CurrentHealth, MaxHealth);
		}

		OnHealthUpdated(CurrentHealth, MaxHealth);
	}
}

void AGDKCharacter::OnRep_CurrentArmour()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		AGDKPlayerController* PC = Cast<AGDKPlayerController>(GetController());
		if (PC)
		{
			PC->UpdateArmourUI(CurrentArmour, MaxArmour);
		}

		OnArmourUpdated(CurrentArmour, MaxArmour);
	}
}

void AGDKCharacter::OnRep_IsRagdoll()
{
	if (bIsRagdoll)
	{
		StartRagdoll();
		DeletionDelegate.BindUFunction(this, FName("DeleteSelf"));
		GetWorldTimerManager().SetTimer(DeletionTimer, DeletionDelegate, 5.0f, false);
	}
}

void AGDKCharacter::DeleteSelf()
{
	if (this->IsValidLowLevel())
	{
		this->Destroy();
	}
}

bool AGDKCharacter::GrantShield(float Value)
{
	if (CurrentArmour < MaxArmour)
	{
		CurrentArmour = FMath::Min(static_cast<int32>(CurrentArmour + Value), MaxArmour);

		return true;
	}

	return false;
}

void AGDKCharacter::RegenerateHealth()
{
	if (CurrentHealth > 0 && CurrentHealth < MaxHealth)
	{
		CurrentHealth = FMath::Min(static_cast<int32>(CurrentHealth + HealthRegenValue), MaxHealth);
	}
}

float AGDKCharacter::TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	TakeDamageCrossServer(Damage, DamageEvent, EventInstigator, DamageCauser);

	return Damage;
}

void AGDKCharacter::TakeDamageCrossServer_Implementation(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return;
	}

	check(DamageCauser);

	Damage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	const AWeapon* DamageSourceWeapon = Cast<AWeapon>(DamageCauser);
	const AGDKCharacter* Killer = Cast<AGDKCharacter>(DamageSourceWeapon->GetWeilder());
	
	int32 ArmourRemoved = FMath::Min(static_cast<int32>(Damage), CurrentArmour);
	CurrentArmour -= ArmourRemoved;
	int32 DamageDealt = FMath::Min(static_cast<int32>(Damage) - ArmourRemoved, CurrentHealth);
	CurrentHealth -= DamageDealt;

	MulticastDamageTaken(DamageCauser->GetActorLocation());

	if (CurrentHealth <= 0)
	{
		Die(Killer);
	}
	else {
		if (GetWorldTimerManager().IsTimerActive(RegenerationHandle))
		{
			GetWorldTimerManager().ClearTimer(RegenerationHandle);
		}
		if (HealthRegenInterval > 0)
		{
			GetWorldTimerManager().SetTimer(RegenerationHandle, this, &AGDKCharacter::RegenerateHealth, HealthRegenInterval, true, HealthRegenCooldown);
		}
	}
}

void AGDKCharacter::MulticastDamageTaken_Implementation(FVector DamageSource)
{
	if (GetNetMode() == NM_Client)
	{
		OnDamageTaken(DamageSource);
	}
}

void AGDKCharacter::OnRep_MetaData()
{
	OnMetaDataUpdated();
}

void AGDKCharacter::SetMetaData(FGDKMetaData NewMetaData)
{
	if (!HasAuthority())
	{
		return;
	}

	MetaData = NewMetaData;
	OnMetaDataUpdated();
}

FString AGDKCharacter::GetPlayerName() const
{
	if (AGDKPlayerState* PS = Cast<AGDKPlayerState>(PlayerState))
	{
		return PS->GetPlayerName();
	}
	return FString("UNKNOWN");
}

void AGDKCharacter::IgnoreMe(AActor* ToIgnore)
{
	GetCapsuleComponent()->IgnoreActorWhenMoving(ToIgnore, true);
}
