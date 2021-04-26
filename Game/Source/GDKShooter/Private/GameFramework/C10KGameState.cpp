 // Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Kismet/GameplayStatics.h"
#include "GameFramework/C10KGameState.h"
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

	FrameTime = 0.0f;
	FrameCount = 0;
}

void AC10KGameState::OnAuthorityGained()
{
	Super::OnAuthorityGained();

	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	UE_LOG(LogC10KGameState, Display, TEXT("%s, %s, Name:[%s]"),
		*SpatialWorkerId, *FString(__FUNCTION__), *this->GetName());
}

void AC10KGameState::OnAuthorityLost()
{
	Super::OnAuthorityLost();

	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	UE_LOG(LogC10KGameState, Display, TEXT("%s, %s, Name:[%s]"),
		*SpatialWorkerId, *FString(__FUNCTION__), *this->GetName());
}

void AC10KGameState::Tick(float DeltaTime)
{
	if (GetGameInstance()->IsDedicatedServerInstance())
	{
		const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();

		FrameCount++;
		FrameTime += DeltaTime;

		if (FrameTime >= 1.0f)
		{
			UE_LOG(LogC10KGameState, Display, TEXT("%s, Server FPS %f"),
				*SpatialWorkerId, FrameCount / FrameTime);

			FrameCount = 0;
			FrameTime = 0.0f;
		}
	}
}

void AC10KGameState::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(AIInfoTimerHandler, this, &AC10KGameState::OutputAIInfos, 10.0f, true, 2.0f);

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

	const FString SpatialWorkerId = GetWorld()->GetGameInstance()->GetSpatialWorkerId();
	UE_LOG(LogC10KGameState, Display, TEXT("%s, %s, Name:[%s]"),
		*SpatialWorkerId, *FString(__FUNCTION__), *this->GetName());
}

void AC10KGameState::OutputAIInfos()
{
	int CharacterCount = 0;
	int DroppedCharacterCount = 0;
	for (TObjectIterator<AGDKCharacter> Itr; Itr; ++Itr)
	{
		CharacterCount++;
		if (Itr->GetActorLocation().Z < 0)
		{
			DroppedCharacterCount++;
		}
	}


	int AAIControllerCount = 0;
	for (TObjectIterator<AAIController> Itr; Itr; ++Itr)
	{
		AAIControllerCount++;
	}

	FString WorkerName = this->GetGameInstance()->IsDedicatedServerInstance() ? TEXT("Server") : TEXT("Client");

	UE_LOG(LogC10KGameState, Warning, TEXT("%s, TotalCharacterCount:[%d], DroppedCharacterCount:[%d], TotalAIControllerCount:[%d]"),
		*WorkerName, CharacterCount, DroppedCharacterCount, AAIControllerCount);
}
