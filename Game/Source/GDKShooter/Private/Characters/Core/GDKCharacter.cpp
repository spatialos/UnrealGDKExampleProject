// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SpatialNetDriver.h"
#include "UnrealNetwork.h"
#include "GDKLogging.h"
#include "Game/GDKPlayerState.h"
#include "Controllers/GDKPlayerController.h"
#include "Weapons/Holdable.h"

// Sets default values
AGDKCharacter::AGDKCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	bIsRagdoll = false;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
}

// Called when the game starts or when spawned
void AGDKCharacter::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->Death.AddDynamic(this, &AGDKCharacter::Die);
}

void AGDKCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGDKCharacter, bIsRagdoll);
}

void AGDKCharacter::Die(const AActor* Killer)
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
		//ShowRespawnScreen();
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

	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	HealthComponent->TakeDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
}
