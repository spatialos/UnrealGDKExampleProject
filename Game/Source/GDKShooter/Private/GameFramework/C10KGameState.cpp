 // Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GameFramework/C10KGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GlobalActor.h"
#include "Characters/GDKCharacter.h"
#include "AIController.h"

DEFINE_LOG_CATEGORY(LogC10KGameState);

AC10KGameState::AC10KGameState()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UClass> NpcClassFinder(TEXT("Class'/Game/Blueprints/NPCs/Character_NPC.Character_NPC_C'"));
	if (NpcClassFinder.Object) {
		NpcClass = (UClass*)NpcClassFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UClass> NpcSpawnerClassFinder(TEXT("Class'/Game/Blueprints/NPCs/NPCSpawner.NPCSpawner_C'"));
	if (NpcSpawnerClassFinder.Object) {
		NpcSpawnerClass = (UClass*)NpcSpawnerClassFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UClass> FireballClassFinder(TEXT("Class'/Game/C10K/FireBall/Fireball.Fireball_C'"));
	if (FireballClassFinder.Object) {
		FireballClass = (UClass*)FireballClassFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UClass> StormClassFinder(TEXT("Class'/Game/C10K/Storm/Storm.Storm_C'"));
	if (StormClassFinder.Object) {
		StormClass = (UClass*)StormClassFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UClass> PoisonClassFinder(TEXT("Class'/Game/C10K/Poison/Poison.Poison_C'"));
	if (PoisonClassFinder.Object) {
		PoisonClass = (UClass*)PoisonClassFinder.Object;
	}

	FrameTime = 0.0f;
	FrameCount = 0;
}

void AC10KGameState::OnAuthorityGained()
{
	Super::OnAuthorityGained();

	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	UE_LOG(LogC10KGameState, Display, TEXT("%s, %s"),
		*SpatialWorkerId, *FString(__FUNCTION__));
}

void AC10KGameState::OnAuthorityLost()
{
	Super::OnAuthorityLost();

	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	UE_LOG(LogC10KGameState, Display, TEXT("%s, %s"),
		*SpatialWorkerId, *FString(__FUNCTION__));
}

void AC10KGameState::Tick(float DeltaTime)
{
	if (GetGameInstance()->IsDedicatedServerInstance())
	{
		const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

		FrameCount++;
		FrameTime += DeltaTime;

		float Fps = FrameCount / FrameTime;

		if (FrameTime >= 0.5f)
		{
			if (!GlobalActor)
			{
				GlobalActor = Cast<AGlobalActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AGlobalActor::StaticClass()));
			}

			if (GlobalActor)
			{
				GlobalActor->UpdateFpsInfo(Fps);
			}

			/*
			UE_LOG(LogC10KGameState, Display, TEXT("%s, Server FPS %f"),
				*SpatialWorkerId, Fps);
			*/

			FrameCount = 0;
			FrameTime = 0.0f;
		}
	}
}

void AC10KGameState::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> OutNpcSpawnerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), NpcSpawnerClass, OutNpcSpawnerActors);

	if (OutNpcSpawnerActors.Num())
	{
		if (HasAuthority())
		{
			GetWorld()->SpawnActor<AGlobalActor>(AGlobalActor::StaticClass(), OutNpcSpawnerActors[0]->GetActorLocation(),
				OutNpcSpawnerActors[0]->GetActorRotation());
		}
	}

	GetWorldTimerManager().SetTimer(TimeUpdateTimerHandle, this, &AC10KGameState::TimestampUpdateTimerHandler, 1.0f, true, 1.0f);

	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	UE_LOG(LogC10KGameState, Display, TEXT("%s, %s"),
		*SpatialWorkerId, *FString(__FUNCTION__));
}

void AC10KGameState::TimestampUpdateTimerHandler()
{
	NowDt = FDateTime::Now();
	NowTs = NowDt.ToUnixTimestamp();
}

