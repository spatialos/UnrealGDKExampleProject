// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "StreamingLevelBoundsAsset.h"
#include "Engine/LevelStreaming.h"

UStreamingLevelBoundsAsset::UStreamingLevelBoundsAsset()
{

}

#if WITH_EDITOR
void UStreamingLevelBoundsAsset::GenerateFromCurrentWorld()
{
	//UWorld* CurrentWorld = GEditor->GetEditorWorldContext().World();
	//const UWorld* CurrentWorld = GetWorld();
	const TArray<ULevelStreaming*> Levels = GWorld->GetStreamingLevels();
	if (Levels.Num() == 0)
	{
		return;
	}

	LevelBounds.Empty();
	for (int32 LevelIndex = 0; LevelIndex < Levels.Num(); LevelIndex++)
	{
		ULevelStreaming* Level = Levels[LevelIndex];
		LevelBounds.Add(Level->PackageNameToLoad.ToString(), Level->GetStreamingVolumeBounds());
	}

}
#endif
