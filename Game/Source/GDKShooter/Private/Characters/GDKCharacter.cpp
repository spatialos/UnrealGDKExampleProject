// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Characters/GDKCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Net/UnrealNetwork.h"
#include "GDKLogging.h"
#include "Characters/GDKSimulatedCharacter.h"
#include "Controllers/GDKPlayerController.h"
#include "Controllers/Components/ControllerEventsComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/Holdable.h"

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

	// static ConstructorHelpers::FObjectFinder<UBlueprint> ItemBlueprint(TEXT("Blueprint'/Game/Blueprints/BlastCubeBlueprint.BlastCubeBlueprint'"));
	static ConstructorHelpers::FObjectFinder<UClass> ItemBlueprint(TEXT("Class'/Game/Blueprints/BlastCubeBlueprint.BlastCubeBlueprint_C'"));
	if (ItemBlueprint.Object) {
		// BlastCubeBlueprint = (UClass*)ItemBlueprint.Object->GeneratedClass;
		BlastCubeBlueprint = (UClass*)ItemBlueprint.Object;
	}
}

// Called when the game starts or when spawned
void AGDKCharacter::BeginPlay()
{
	Super::BeginPlay();

	EquippedComponent->HoldableUpdated.AddDynamic(this, &AGDKCharacter::OnEquippedUpdated);
	GDKMovementComponent->SprintingUpdated.AddDynamic(EquippedComponent, &UEquippedComponent::SetIsSprinting);

	// yunjie: binding hit event
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AGDKCharacter::OnCapsuleCompHit);

	if (GetWorld()->GetGameInstance()->IsSimulatedPlayer())
	{
		
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

	// yunjie: testing codes
	PlayerInputComponent->BindAction("0", IE_Pressed, this, &AGDKCharacter::ServerSpawnBlastActors);
	PlayerInputComponent->BindAction("1", IE_Pressed, this, &AGDKCharacter::ClientPrintCurrentBlastInfos);
	PlayerInputComponent->BindAction("2", IE_Pressed, this, &AGDKCharacter::ServerPrintCurrentBlastInfos);
	PlayerInputComponent->BindAction("3", IE_Pressed, this, &AGDKCharacter::ServerStartTimerToBlast);
	PlayerInputComponent->BindAction("4", IE_Pressed, this, &AGDKCharacter::PrintSimBotsCount);
	PlayerInputComponent->BindAction("5", IE_Pressed, this, &AGDKCharacter::ServerSetAIMode);
	PlayerInputComponent->BindAction("6", IE_Pressed, this, &AGDKCharacter::ServerPrintBlastStats);
	PlayerInputComponent->BindAction("7", IE_Pressed, this, &AGDKCharacter::SetDebrisLifetime_Quick);
	PlayerInputComponent->BindAction("8", IE_Pressed, this, &AGDKCharacter::SetDebrisLifetime_Normal);
	PlayerInputComponent->BindAction("9", IE_Pressed, this, &AGDKCharacter::SetDebrisLifetime_Forever);
	PlayerInputComponent->BindAction("+", IE_Pressed, this, &AGDKCharacter::ServerIncreaseBlastActorCountPerSecond);
	PlayerInputComponent->BindAction("-", IE_Pressed, this, &AGDKCharacter::ServerDecreaseBlastActorCountPerSecond);
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
	// yunjie: temporarily commented
	return;
	
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

void AGDKCharacter::OnCapsuleCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		// if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("I Hit: %s"), *OtherActor->GetName()));

		FString WorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
		FString WorkerType = GetWorld()->GetGameInstance()->GetSpatialWorkerType().ToString();
		FString WorkerLabel = GetWorld()->GetGameInstance()->GetSpatialWorkerLabel();
		FString IsServer = GetWorld()->GetGameInstance()->IsDedicatedServerInstance() ? "YES" : "NO";
		FString Authority;

		if (this->GetOwner())
		{
			Authority = this->GetOwner()->HasAuthority() ? "YES" : "NO";
		}

		/*
		UE_LOG(LogGDK, Warning, TEXT("%s - WorkerId:[%s] WorkerType:[%s] WorkerLabel:[%s] Name:[%s] OtherActorName:[%s] IsServer:[%s] Authority:[%s]"),
			*FString(__FUNCTION__), *WorkerId, *WorkerType, *WorkerLabel, *GetFName().ToString(), *OtherActor->GetFName().ToString(), *IsServer, *Authority);
			*/
	}
}

void AGDKCharacter::PrintCurrentBlastInfos(const FString& Func)
{
	FString FuncName = Func.IsEmpty() ? FString(__FUNCTION__) : Func;

	TArray<AActor*> FoundBlastActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), REAL_BLAST_MESH_ACTOR::StaticClass(), FoundBlastActors);

	for (int i = 0; i < FoundBlastActors.Num(); ++i)
	{
		REAL_BLAST_MESH_ACTOR* BlastActor = Cast<REAL_BLAST_MESH_ACTOR>(FoundBlastActors[i]);
		if (BlastActor)
		{
			REAL_BLAST_MESH_COMPONENT* BlastComp = Cast<REAL_BLAST_MESH_COMPONENT>(BlastActor->GetBlastMeshComponent());
			if (BlastComp)
			{
				UE_LOG(LogGDK, Warning, TEXT("%s - Index:[%d] Fracture Count:[%d] DebrisCount:[%d]"), *FuncName, i, BlastComp->CanBeFracturedCount(), BlastComp->GetDebrisCount());
			}
		}
	}

	UE_LOG(LogGDK, Warning, TEXT("%s - BlastActor Total Count:[%d]"), *FuncName, FoundBlastActors.Num());
}

void AGDKCharacter::ClientPrintCurrentBlastInfos()
{
	PrintCurrentBlastInfos(FString(__FUNCTION__));
}

void AGDKCharacter::ServerPrintCurrentBlastInfos_Implementation()
{
	PrintCurrentBlastInfos(FString(__FUNCTION__));

	TArray<AActor*> FoundBlastActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), REAL_BLAST_MESH_ACTOR::StaticClass(), FoundBlastActors);
	if (FoundBlastActors.Num() > 0)
	{
		REAL_BLAST_MESH_ACTOR* BlastActor = Cast<REAL_BLAST_MESH_ACTOR>(FoundBlastActors[0]);
		if (BlastActor)
		{
			if (!BlastActor->HasAuthority())
			{
				BlastActor->CrossServerPrintCurrentBlastInfos();
			}
		}
	}
}

void AGDKCharacter::BlastTimerEvent()
{
	FString IsServer = GetGameInstance()->IsDedicatedServerInstance() ? "YES" : "NO";
	UE_LOG(LogGDK, Warning, TEXT("%s - IsServer:[%s]"), *FString(__FUNCTION__), *IsServer);

	int BlastCount = 0;
	TArray<AActor*> FoundBlastActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), REAL_BLAST_MESH_ACTOR::StaticClass(), FoundBlastActors);
	for (int i = 0; i < FoundBlastActors.Num(); ++i)
	{
		REAL_BLAST_MESH_ACTOR* TmpBlastActor = Cast<REAL_BLAST_MESH_ACTOR>(FoundBlastActors[i]);
		if (TmpBlastActor)
		{
			if (TmpBlastActor->GetBlastCount() >= 1)
			{
				continue;
			}

			REAL_BLAST_MESH_COMPONENT* BlastComp = Cast<REAL_BLAST_MESH_COMPONENT>(TmpBlastActor->GetBlastMeshComponent());
			if (BlastComp)
			{
				/** yunjie: this fractured count is not the correct one on offloading worker, so don't rely on this
				if (BlastComp->CanBeFracturedCount() <= 0)
				{
					continue;
				}
				*/

				// yunjie: one time damage may not destruct the whole blast actor, so do this few times to make sure it's entirely exploded
				for (int32 damageCount = 0; damageCount < 2; damageCount++)
				{
					TmpBlastActor->CrossServerApplyDamage(TmpBlastActor->GetActorLocation(), 200, 300, 1000, 500, true);
				}

				TmpBlastActor->IncBlastCount();

				UE_LOG(LogGDK, Warning, TEXT("%s - Found one to blast, index:[%d]"), *FString(__FUNCTION__), i);

				if (++BlastCount >= BlastActorCountPerSecond)
				{
					break;
				}

				REAL_BLAST_MESH_COMPONENT* Comp = Cast<REAL_BLAST_MESH_COMPONENT>(TmpBlastActor->GetBlastMeshComponent());
				if (Comp)
				{
				}
			}
		}
	}

	BlastDelegate.BindUFunction(this, FName("BlastTimerEvent"));
	GetWorldTimerManager().SetTimer(BlastTimer, BlastDelegate, 1, false);
}

void AGDKCharacter::ServerStartTimerToBlast_Implementation()
{
	if (!BlastTimer.IsValid())
	{
		UE_LOG(LogGDK, Warning, TEXT("%s - Set Timer"), *FString(__FUNCTION__));
		BlastDelegate.BindUFunction(this, FName("BlastTimerEvent"));
		GetWorldTimerManager().SetTimer(BlastTimer, BlastDelegate, 1, false);
	}
	else
	{
		UE_LOG(LogGDK, Warning, TEXT("%s - Clear Timer"), *FString(__FUNCTION__));
		GetWorldTimerManager().ClearTimer(BlastTimer);
	}
}

void AGDKCharacter::ServerIncreaseBlastActorCountPerSecond_Implementation()
{
	BlastActorCountPerSecond++;

	UE_LOG(LogGDK, Warning, TEXT("%s - Count:[%d]"), *FString(__FUNCTION__), BlastActorCountPerSecond);
}

void AGDKCharacter::ServerDecreaseBlastActorCountPerSecond_Implementation()
{
	if (--BlastActorCountPerSecond <= 0)
	{
		BlastActorCountPerSecond = 1;
	}

	UE_LOG(LogGDK, Warning, TEXT("%s - Count:[%d]"), *FString(__FUNCTION__), BlastActorCountPerSecond);
}


void AGDKCharacter::ServerSpawnBlastActors_Implementation()
{
	UE_LOG(LogGDK, Warning, TEXT("%s"), *FString(__FUNCTION__));

	int32 CountLimitation = INT_MAX;
	CountLimitation = 200;
	static int32 AccCount = 0;
	int32 CurrentCount = 0;
	int32 CurrentSkipIndex = 0;

	int MatrixAWidth = 32;
	int MatrixALength = 26;

	int MatrixBWidth = 32;
	int MatrixBLength = 26;

	int MatrixACubeCount = MatrixAWidth * MatrixALength;
	int MatrixBCubeCount = MatrixBWidth * MatrixBLength;
	int TotalCubeCount = MatrixACubeCount + MatrixBCubeCount;

	bool bCenterCubes = true;

	// yunjie: destroy all blast actors
	TArray<AActor*> FoundBlastActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), REAL_BLAST_MESH_ACTOR::StaticClass(), FoundBlastActors);

	if (bCenterCubes)
	{
		int MatrixCenterWidth = 22;
		int MatrixCenterLength = 22;
		int MatrixCenterTotalCount = MatrixCenterWidth * MatrixCenterLength;
		TotalCubeCount = MatrixCenterTotalCount;

		if (FoundBlastActors.Num() >= TotalCubeCount)
		{
			UE_LOG(LogGDK, Warning, TEXT("%s - start to destroy existing blast actors"), *FString(__FUNCTION__));

			for (int i = 0; i < FoundBlastActors.Num(); ++i)
			{
				REAL_BLAST_MESH_ACTOR* BlastActor = Cast<REAL_BLAST_MESH_ACTOR>(FoundBlastActors[i]);
				if (BlastActor->HasAuthority())
				{
					BlastActor->Destroy();
				}
				else
				{
					BlastActor->CrossServerDestroyAllActors();
					break;
				}
			}

			AccCount = 0;
		}
		else if (FoundBlastActors.Num() < TotalCubeCount)
		{
			UE_LOG(LogGDK, Warning, TEXT("%s - start to spawn maxtrix center blast actors"), *FString(__FUNCTION__));

			int32 y = 1690;
			for (int32 i = 0; i < MatrixCenterWidth; ++i)
			{
				int32 x = 1720;
				for (int32 j = 0; j < MatrixCenterLength; ++j)
				{
					if (CurrentSkipIndex++ >= AccCount)
					{
						FVector v = FVector(x, y, 60);
						// ATestBlastMeshActor* BlastActor = GetWorld()->SpawnActor<ATestBlastMeshActor>(ATestBlastMeshActor::StaticClass(), v, FRotator::ZeroRotator);
						ATestBlastMeshActor* BlastActor = GetWorld()->SpawnActor<ATestBlastMeshActor>(BlastCubeBlueprint, v, FRotator::ZeroRotator);
						++AccCount;
						if (++CurrentCount >= CountLimitation)
						{
							goto RETURN_LABEL;
						}
					}

					x -= 300;
				}
				y -= 300;
			}
		}

	}
	else
	{
		if (FoundBlastActors.Num() >= TotalCubeCount)
		{
			UE_LOG(LogGDK, Warning, TEXT("%s - start to destroy existing blast actors"), *FString(__FUNCTION__));

			for (int i = 0; i < FoundBlastActors.Num(); ++i)
			{
				REAL_BLAST_MESH_ACTOR* BlastActor = Cast<REAL_BLAST_MESH_ACTOR>(FoundBlastActors[i]);
				if (BlastActor->HasAuthority())
				{
					BlastActor->Destroy();
				}
				else
				{
					BlastActor->CrossServerDestroyAllActors();
					break;
				}
			}

			AccCount = 0;
		}
		else if (FoundBlastActors.Num() < MatrixACubeCount)
		{
			UE_LOG(LogGDK, Warning, TEXT("%s - start to spawn maxtrix A blast actors"), *FString(__FUNCTION__));

			int32 y = 3680;
			for (int32 i = 0; i < 32; ++i)
			{
				int32 x = 60;
				for (int32 j = 0; j < 26; ++j)
				{
					if (CurrentSkipIndex++ >= AccCount)
					{
						FVector v = FVector(x, y, 60);
						// ATestBlastMeshActor* BlastActor = GetWorld()->SpawnActor<ATestBlastMeshActor>(ATestBlastMeshActor::StaticClass(), v, FRotator::ZeroRotator);
						ATestBlastMeshActor* BlastActor = GetWorld()->SpawnActor<ATestBlastMeshActor>(BlastCubeBlueprint, v, FRotator::ZeroRotator);
						++AccCount;
						if (++CurrentCount >= CountLimitation)
						{
							goto RETURN_LABEL;
						}
					}

					x -= 120;
				}
				y -= 120;
			}
		}
		else
		{
			UE_LOG(LogGDK, Warning, TEXT("%s - start to spawn maxtrix B blast actors"), *FString(__FUNCTION__));

			if (AccCount == MatrixACubeCount)
			{
				AccCount = 0;
			}

			int y = 280;
			for (int32 i = 0; i < 32; ++i)
			{
				int32 x = 3660;
				for (int32 j = 0; j < 26; ++j)
				{
					if (CurrentSkipIndex++ >= AccCount)
					{
						FVector v = FVector(x, y, 60);
						// ATestBlastMeshActor* BlastActor = GetWorld()->SpawnActor<ATestBlastMeshActor>(ATestBlastMeshActor::StaticClass(), v, FRotator::ZeroRotator);
						ATestBlastMeshActor* BlastActor = GetWorld()->SpawnActor<ATestBlastMeshActor>(BlastCubeBlueprint, v, FRotator::ZeroRotator);
						++AccCount;
						if (++CurrentCount >= CountLimitation)
						{
							goto RETURN_LABEL;
						}
					}

					x -= 120;
				}
				y -= 120;
			}
		}
	}

RETURN_LABEL:
	return;
}

void AGDKCharacter::SetDebrisLifetime_Quick()
{
	REAL_BLAST_MESH_ACTOR::SetAllDebrisLifeTime(GetWorld(), DEBRIS_LIFETIME_QUICK_MIN, DEBRIS_LIFETIME_QUICK_MAX);
	ServerSetDebrisLifetime(DEBRIS_LIFETIME_QUICK_MIN, DEBRIS_LIFETIME_QUICK_MAX);
}

void AGDKCharacter::SetDebrisLifetime_Normal()
{
	REAL_BLAST_MESH_ACTOR::SetAllDebrisLifeTime(GetWorld(), DEBRIS_LIFETIME_NORMAL_MIN, DEBRIS_LIFETIME_NORMAL_MAX);
	ServerSetDebrisLifetime(DEBRIS_LIFETIME_NORMAL_MIN, DEBRIS_LIFETIME_NORMAL_MAX);
}

void AGDKCharacter::SetDebrisLifetime_Forever()
{
	REAL_BLAST_MESH_ACTOR::SetAllDebrisLifeTime(GetWorld(), DEBRIS_LIFETIME_FOREVER_MIN, DEBRIS_LIFETIME_FOREVER_MAX);
	ServerSetDebrisLifetime(DEBRIS_LIFETIME_FOREVER_MIN, DEBRIS_LIFETIME_FOREVER_MAX);
}

void AGDKCharacter::ServerSetDebrisLifetime_Implementation(int32 min, int32 max)
{
	UE_LOG(LogGDK, Warning, TEXT("%s - min:[%d], max:[%d]"), *FString(__FUNCTION__), min, max);

	REAL_BLAST_MESH_ACTOR::SetAllDebrisLifeTime(GetWorld(), min, max);

	TArray<AActor*> FoundBlastActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), REAL_BLAST_MESH_ACTOR::StaticClass(), FoundBlastActors);
	if (FoundBlastActors.Num() > 0)
	{
		REAL_BLAST_MESH_ACTOR* BlastActor = Cast<REAL_BLAST_MESH_ACTOR>(FoundBlastActors[0]);
		if (BlastActor)
		{
			if (!BlastActor->HasAuthority())
			{
				BlastActor->CrossServerSetAllDebrisLifetime(min, max);
			}
		}
	}

}

void AGDKCharacter::ServerApplyDamage_Implementation(REAL_BLAST_MESH_ACTOR* BlastActor, FVector Origin, float MinRadius, float MaxRadius, float Damage, float ImpulseStrength, bool bImpulseVelChange)
{
	BlastActor->CrossServerApplyDamage(Origin, MinRadius, MaxRadius, Damage, ImpulseStrength, bImpulseVelChange);
}


void AGDKCharacter::ServerPrintBlastStats_Implementation()
{
	TArray<AActor*> FoundBlastActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), REAL_BLAST_MESH_ACTOR::StaticClass(), FoundBlastActors);
	if (FoundBlastActors.Num() > 0)
	{
		REAL_BLAST_MESH_ACTOR* BlastActor = Cast<REAL_BLAST_MESH_ACTOR>(FoundBlastActors[0]);
		if (BlastActor)
		{
			if (!BlastActor->HasAuthority())
			{
				BlastActor->CrossServerPrintBlastStats();
			}
		}
	}
}


void AGDKCharacter::ServerSetAIMode_Implementation()
{
	MulticastAIMode();
}

void AGDKCharacter::ClientSetAIMode_Implementation(int AIMode_)
{
	AIMode = AIMode_;
	UE_LOG(LogGDK, Warning, TEXT("%s - AIMode:[%d]"), *FString(__FUNCTION__), AIMode);
}

void AGDKCharacter::MulticastAIMode()
{
	if (++AIMode >= (int)AIM_MAX)
	{
		AIMode = (int)AIM_PAUSE;
	}

	TArray<AActor*> FoundBlastActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGDKSimulatedCharacter::StaticClass(), FoundBlastActors);
	for (int i = 0; i < FoundBlastActors.Num(); ++i)
	{
		if (FoundBlastActors[i])
		{
			auto simChar = Cast<AGDKSimulatedCharacter>(FoundBlastActors[i]);
			simChar->ClientSetAIMode(AIMode);
		}
	}
}

float AGDKCharacter::GetBurstDuration()
{
	if (AIM_LOW_FREQUENCY_FIRE == AIMode)
	{
		return 0.1;
	}
	else if (AIM_HIGH_FREQUENCY_FIRE == AIMode)
	{
		return 0.4;
	}

	return 0.0;
}

void AGDKCharacter::PrintSimBotsCount()
{
	TArray<AActor*> FoundBlastActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGDKSimulatedCharacter::StaticClass(), FoundBlastActors);
	UE_LOG(LogGDK, Warning, TEXT("%s - SimBots Count:[%d]"), *FString(__FUNCTION__), FoundBlastActors.Num());
}

