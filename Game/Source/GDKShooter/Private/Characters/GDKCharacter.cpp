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

AGDKCharacter::AGDKCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGDKMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	EquippedComponent = CreateDefaultSubobject<UEquippedComponent>(TEXT("Equipment"));
	MetaDataComponent = CreateDefaultSubobject<UMetaDataComponent>(TEXT("MetaData"));
	GDKMovementComponent = Cast<UGDKMovementComponent>(GetCharacterMovement());
}

// Called when the game starts or when spawned
void AGDKCharacter::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->AuthoritativeDeath.AddDynamic(this, &AGDKCharacter::Die);
	//HealthComponent->Death.AddDynamic(this, &AGDKCharacter::StartRagdoll);
	EquippedComponent->HoldableUpdated.AddDynamic(this, &AGDKCharacter::OnEquippedUpdated);
	GDKMovementComponent->SprintingUpdated.AddDynamic(EquippedComponent, &UEquippedComponent::SetIsSprinting);
	MetaDataComponent->MetaDataUpdated.AddDynamic(EquippedComponent, &UEquippedComponent::SpawnStarterTemplates);
}

void AGDKCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGDKCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGDKCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction<FBoolean>("Sprint", IE_Pressed, GDKMovementComponent, &UGDKMovementComponent::SetWantsToSprint, true);
	PlayerInputComponent->BindAction<FBoolean>("Sprint", IE_Released, GDKMovementComponent, &UGDKMovementComponent::SetWantsToSprint, false);
	PlayerInputComponent->BindAction<FBoolean>("Crouch", IE_Pressed, this, &AGDKCharacter::Crouch, true);
	PlayerInputComponent->BindAction<FBoolean>("Crouch", IE_Released, this, &AGDKCharacter::UnCrouch, true);

	PlayerInputComponent->BindAction("Primary", IE_Pressed, EquippedComponent, &UEquippedComponent::StartPrimaryUse);
	PlayerInputComponent->BindAction("Primary", IE_Released, EquippedComponent, &UEquippedComponent::StopPrimaryUse);
	PlayerInputComponent->BindAction("Secondary", IE_Pressed, EquippedComponent, &UEquippedComponent::StartSecondaryUse);
	PlayerInputComponent->BindAction("Secondary", IE_Released, EquippedComponent, &UEquippedComponent::StopSecondaryUse);

	PlayerInputComponent->BindAction< FHoldableSelection>("1", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 0);
	PlayerInputComponent->BindAction< FHoldableSelection>("2", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 1);
	PlayerInputComponent->BindAction< FHoldableSelection>("3", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 2);
	PlayerInputComponent->BindAction< FHoldableSelection>("4", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 3);
	PlayerInputComponent->BindAction< FHoldableSelection>("5", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 4);
	PlayerInputComponent->BindAction< FHoldableSelection>("6", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 5);
	PlayerInputComponent->BindAction< FHoldableSelection>("7", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 6);
	PlayerInputComponent->BindAction< FHoldableSelection>("8", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 7);
	PlayerInputComponent->BindAction< FHoldableSelection>("9", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 8);
	PlayerInputComponent->BindAction< FHoldableSelection>("0", IE_Pressed, EquippedComponent, &UEquippedComponent::ServerRequestEquip, 9);
	PlayerInputComponent->BindAction("QuickToggle", IE_Pressed, EquippedComponent, &UEquippedComponent::QuickToggle);
	PlayerInputComponent->BindAction("ToggleMode", IE_Pressed, EquippedComponent, &UEquippedComponent::ToggleMode);
	PlayerInputComponent->BindAction("ScrollUp", IE_Pressed, EquippedComponent, &UEquippedComponent::ScrollUp);
	PlayerInputComponent->BindAction("ScrollDown", IE_Pressed, EquippedComponent, &UEquippedComponent::ScrollDown);
}

void AGDKCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AGDKCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AGDKCharacter::OnEquippedUpdated_Implementation(AHoldable* Holdable)
{
	if (Holdable)
	{
		Holdable->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Holdable->GetActiveSocket());
	}
}

void AGDKCharacter::Die(const AActor* Killer)
{
	TearOff();

	AGDKPlayerController* PC = Cast<AGDKPlayerController>(GetController());
	if (PC)
	{
		PC->KillCharacter(Killer);
	}

	DeletionDelegate.BindUFunction(this, FName("DeleteSelf"));
	GetWorldTimerManager().SetTimer(DeletionTimer, DeletionDelegate, 5.0f, false);
}

void AGDKCharacter::TornOff()
{
	StartRagdoll();
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

	DeletionDelegate.BindUFunction(this, FName("DeleteSelf"));
	GetWorldTimerManager().SetTimer(DeletionTimer, DeletionDelegate, 5.0f, false);
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
	if (HasAuthority())
	{
		float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
		HealthComponent->TakeDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
	}

}
