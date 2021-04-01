// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "StreamingLevelBoundsActor.h"
#include "Engine/LevelBounds.h"

// Sets default values
AStreamingLevelBoundsActor::AStreamingLevelBoundsActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AStreamingLevelBoundsActor::BeginPlay()
{
	Super::BeginPlay();
	
}

#if WITH_EDITOR
void AStreamingLevelBoundsActor::GenerateFromCurrentWorld()
{
	const UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld == nullptr)
		CurrentWorld = GWorld;
	const TArray<ULevelStreaming*> Levels = CurrentWorld->GetStreamingLevels();
	if (Levels.Num() == 0)
	{
		return;
	}

	StreamingVolumeBounds.Empty();
	for (int32 LevelIndex = 0; LevelIndex < Levels.Num(); LevelIndex++)
	{
		ULevelStreaming* Level = Levels[LevelIndex];
		FString LevelPath = Level->GetWorldAssetPackageName();
		StreamingVolumeBounds.Add(LevelPath, Level->GetStreamingVolumeBounds());

		if (const ULevel* LoadedLevel = Level->GetLoadedLevel())
		{
			if (LoadedLevel->LevelBoundsActor != nullptr)
			{
				LevelBounds.Add(LevelPath, LoadedLevel->LevelBoundsActor->GetComponentsBoundingBox());
			}
		}
	}
}
#endif
