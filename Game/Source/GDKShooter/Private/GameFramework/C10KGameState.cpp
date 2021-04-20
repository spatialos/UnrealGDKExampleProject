 // Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GameFramework/C10KGameState.h"
#include "Characters/GDKCharacter.h"
#include "AIController.h"

DEFINE_LOG_CATEGORY(LogC10KGameState);

AC10KGameState::AC10KGameState()
{
	static ConstructorHelpers::FObjectFinder<UClass> NpcClassFinder(TEXT("Class'/Game/Blueprints/NPCs/Character_NPC.Character_NPC_C'"));
	if (NpcClassFinder.Object) {
		NpcClass = (UClass*)NpcClassFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UClass> NpcSpawnerClassFinder(TEXT("Class'/Game/Blueprints/NPCs/NPCSpawner.NPCSpawner_C'"));
	if (NpcSpawnerClassFinder.Object) {
		NpcSpawnerClass = (UClass*)NpcSpawnerClassFinder.Object;
	}
}

void AC10KGameState::BeginPlay()
{
	AGameStateBase::BeginPlay();
	GetWorldTimerManager().SetTimer(AIInfoTimerHandler, this, &AC10KGameState::OutputAIInfos, 10.0f, true, 2.0f);
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
