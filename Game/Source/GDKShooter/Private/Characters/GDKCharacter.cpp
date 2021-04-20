// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Characters/GDKCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Net/UnrealNetwork.h"
#include "GDKLogging.h"
#include "Controllers/GDKPlayerController.h"
#include "AIModule/Classes/AIController.h"
#include "Controllers/Components/ControllerEventsComponent.h"
#include "Weapons/Holdable.h"
#include "GameFramework/C10KGameState.h"

AGDKCharacter::AGDKCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGDKMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	EquippedComponent = CreateDefaultSubobject<UEquippedComponent>(TEXT("Equipment"));
	MetaDataComponent = CreateDefaultSubobject<UMetaDataComponent>(TEXT("MetaData"));
	TeamComponent = CreateDefaultSubobject<UTeamComponent>(TEXT("Team"));
	GDKMovementComponent = Cast<UGDKMovementComponent>(GetCharacterMovement());
}

// Called when the game starts or when spawned
void AGDKCharacter::BeginPlay()
{
	Super::BeginPlay();

	EquippedComponent->HoldableUpdated.AddDynamic(this, &AGDKCharacter::OnEquippedUpdated);
	GDKMovementComponent->SprintingUpdated.AddDynamic(EquippedComponent, &UEquippedComponent::SetIsSprinting);

	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetNetDriver());

	if (this->GetGameInstance()->IsDedicatedServerInstance() && NetDriver->SpatialPlatformCoordinator->CheckPlatformSwitch(false))
	{
		Cast<USpatialNetDriver>(this->GetNetDriver())->SpatialPlatformCoordinator->SendReadyStatus();
	}
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

	PlayerInputComponent->BindAxis("TurnRate", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APawn::AddControllerPitchInput);
    
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

	PlayerInputComponent->BindAction("+", IE_Pressed, this, &AGDKCharacter::ServerSpawnAIEntities);
	PlayerInputComponent->BindAction("-", IE_Pressed, this, &AGDKCharacter::ServerDestroyAIEntities);
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

void AGDKCharacter::StartRagdoll_Implementation()
{
	// Disable capsule collision and disable movement.
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule == nullptr)
	{
		UE_LOG(LogGDK, Error, TEXT("Invalid capsule component on character %s"), *this->GetName());
		return;
	}

	Capsule->SetSimulatePhysics(false);
	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// Enable mesh collision and physics.
	USkeletalMeshComponent* MeshComponent = GetMesh();
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	MeshComponent->SetCollisionProfileName(FName(TEXT("Ragdoll")));
	MeshComponent->SetSimulatePhysics(true);

	// Gather list of child components of the capsule.
	TArray<USceneComponent*> ComponentsToMove;
	int NumChildren = Capsule->GetNumChildrenComponents();
	for (int i = 0; i < NumChildren; ++i)
	{
		USceneComponent* Component = Capsule->GetChildComponent(i);
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

	if (this->IsValidLowLevel() && RagdollLifetime >= 0.f)
	{
		DeletionDelegate.BindUFunction(this, FName("DeleteSelf"));
		GetWorldTimerManager().SetTimer(DeletionTimer, DeletionDelegate, RagdollLifetime, false);
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
	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	HealthComponent->TakeDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);

}

FGenericTeamId AGDKCharacter::GetGenericTeamId() const
{
	return TeamComponent->GetTeam();
}

bool AGDKCharacter::CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor) const
{
	int32 PositiveHits = 0;

	if (HealthComponent->GetCurrentHealth() <= 0)
	{
		return 0;
	}

	bool bHasSeen = false;

	for (int i = 0; i < LineOfSightSockets.Num(); i++)
	{
		FVector Target = GetMesh()->GetSocketLocation(LineOfSightSockets[i]);
		FHitResult HitResult;
		const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, ObserverLocation, Target
			, LineOfSightCollisionChannel.GetValue()
			, FCollisionQueryParams(SCENE_QUERY_STAT(AILineOfSight), true, IgnoreActor));

		if (bHit == false || (HitResult.Actor.IsValid() && HitResult.Actor->IsOwnedBy(this)))
		{
			if (!bHasSeen)
			{
				OutSeenLocation = Target;
				bHasSeen = true;
			}
			PositiveHits++;
		}
	}
	NumberOfLoSChecksPerformed = LineOfSightSockets.Num();
	OutSightStrength = (float)PositiveHits / (float)NumberOfLoSChecksPerformed;
	return PositiveHits > 0;
}

void AGDKCharacter::ClientMovementReset_Implementation()
{
	GetCharacterMovement()->ResetPredictionData_Server();
}

void AGDKCharacter::ServerSpawnAIEntities_Implementation()
{
	UE_LOG(LogGDK, Warning, TEXT("%s"), *(FString(__FUNCTION__)));

	AC10KGameState *GameState = Cast<AC10KGameState>(GetWorld()->GetGameState());
	UClass* NpcClass = GameState->NpcClass;
	UClass* NpcSpawnerClass = GameState->NpcSpawnerClass;

	TArray<AActor*> OutPlayerStartActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), OutPlayerStartActors);

	if (!OutPlayerStartActors.Num())
	{
		return;
	}

	TArray<AActor*> OutNpcSpawnerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), NpcSpawnerClass, OutNpcSpawnerActors);

	if (!OutNpcSpawnerActors.Num())
	{
		return;
	}

	for (int Index = 0; Index < AI_SPAWN_COUNT_PER_BATCH; ++Index)
	{
		int RandIdx = FMath::RandRange(0, OutPlayerStartActors.Num() - 1);
		APlayerStart * PlayerStart = Cast<APlayerStart>(OutPlayerStartActors[RandIdx]);
		if (!PlayerStart)
		{
			continue;
		}

		FActorSpawnParameters SpawnParam;
		SpawnParam.Owner = OutNpcSpawnerActors[0];

		AGDKCharacter* Character = GetWorld()->SpawnActor<AGDKCharacter>(NpcClass, PlayerStart->GetActorLocation(),
			PlayerStart->GetActorRotation(), SpawnParam);
		if (!Character)
		{
			continue;
		}

		AAIController* AIController = GetWorld()->SpawnActor<AAIController>(Character->AIControllerClass, PlayerStart->GetActorLocation(),
			PlayerStart->GetActorRotation(), SpawnParam);
		if (!AIController)
		{
			Character->Destroy();
			continue;
		}

		AIController->Possess(Character);
	}
}

void AGDKCharacter::ServerDestroyAIEntities_Implementation()
{
	int CharacterCount = 0;
	int AIControllerCount = 0;

	AC10KGameState *GameState = Cast<AC10KGameState>(GetWorld()->GetGameState());
	UClass* NpcClass = GameState->NpcClass;

	for (TObjectIterator<AGDKCharacter> Itr; Itr; ++Itr)
	{
		if (Itr->GetClass()->IsChildOf(NpcClass))
		{
			AController* AIController = Itr->Controller;
			if (AIController)
			{
				AIController->UnPossess();
				AIController->Destroy();
				++AIControllerCount;
			}

			// Itr->UnPossessed();
			Itr->Destroy();
			if (++CharacterCount >= AI_SPAWN_COUNT_PER_BATCH)
			{
				break;
			}
		}
	}

	// yunjie: as the garbage system of unreal won't immediately free the actor after calling destroy
	//	so just force to call the GC both on server and client side
	GEngine->ForceGarbageCollection();
	ClientForceGarbageCollection();

	UE_LOG(LogGDK, Warning, TEXT("%s - destroyed character count:[%d], destroyed aicontroller count:[%d]"),
		*(FString(__FUNCTION__)), CharacterCount, AIControllerCount);
}

void AGDKCharacter::ClientForceGarbageCollection_Implementation()
{
	GEngine->ForceGarbageCollection();
	UE_LOG(LogGDK, Warning, TEXT("%s"), *(FString(__FUNCTION__)));
}

