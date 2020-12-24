// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StreamingLevelBoundsActor.generated.h"

UCLASS()
class GDKSHOOTER_API AStreamingLevelBoundsActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStreamingLevelBoundsActor();

	UPROPERTY(Category = "StreamingLevelBounds", EditAnywhere)
	TMap<FString, FBox> StreamingVolumeBounds;
	UPROPERTY(Category = "StreamingLevelBounds", EditAnywhere)
	TMap<FString, FBox> LevelBounds;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

#if WITH_EDITOR
	UFUNCTION(Category = "StreamingLevelBoundsAsset", BlueprintCallable, CallInEditor)
	void GenerateFromCurrentWorld();
#endif

};
