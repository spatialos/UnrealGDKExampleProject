// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StreamingLevelBoundsAsset.generated.h"

/**
 * 
 */
UCLASS()
class GDKSHOOTER_API UStreamingLevelBoundsAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UStreamingLevelBoundsAsset();

	UPROPERTY(Category = "StreamingLevelBoundsAsset", EditAnywhere)
	TMap<FString, FBox> LevelBounds;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Category = "StreamingLevelBoundsAsset", EditAnywhere, meta = (AllowedClasses = "World"))
	FSoftObjectPath MapForGeneration;
#endif
#if WITH_EDITOR
	//TWeakObjectPtr<UWorld> CurrentWorld;

	UFUNCTION(Category = "StreamingLevelBoundsAsset", BlueprintCallable, CallInEditor)
	void GenerateFromCurrentWorld();
#endif
};
