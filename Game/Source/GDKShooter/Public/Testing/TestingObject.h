// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestingObject.generated.h"

UCLASS(Blueprintable)
class GDKSHOOTER_API ATestingObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestingObject(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Category = Meshes, VisibleAnywhere)
	UStaticMeshComponent* CubeMesh;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool HasBeenCheckedOutByClient = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
